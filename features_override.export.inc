<?php

/**
 * @file
 * Helper function to export features overrides.
 */

/**
 * Parses the identifier into indivudal parts.
 */
function features_override_parse_identifier($identifier) {
  // Identifier is <comonent>.<element>.<keys> or <component>.<element>.
  // Need special handling since key is serialized so can't explode.
  $first_period = strpos($identifier, '.');
  $component = substr($identifier, 0, $first_period);
  if ($second_period = strpos($identifier, '.', $first_period + 1)) {
    $element = substr($identifier, $first_period + 1, $second_period - $first_period - 1);
    $keys = substr($identifier, $second_period + 1);
  }
  else {
    $element = substr($identifier, $first_period + 1);
    $keys = FALSE;
  }
  return array($component, $element, $keys);
}

/**
 * Makes a distinct string key from an array of keys.
 *
 * @param $keys
 *   An array of keys.
 */
function features_override_make_key($keys) {
  $return_keys = array();
  foreach ($keys as $key) {
    $return_keys[] = $key['key'];
  }
  return implode('|', $return_keys);
}

/**
 * Calculautes what overrides exist for by component/element.
 *
 * @param $component_key
 *   A component key that's defined via hook_features_api.
 * @param $element_key
 *   A key identifieing an element that's been overriden.
 */
function features_override_get_overrides($component_key = FALSE, $element_key = FALSE, $reset = FALSE) {
  static $cache;
  if (!isset($cache) || $reset) {
    $cache = array();

    foreach (features_get_components() as $component => $info) {
      if (empty($info['default_hook']) || $component == 'features_override') {
        continue;
      }
      $modules = array();
      foreach (array_filter(features_get_component_map($component)) as $element => $element_modules) {
        $modules = array_merge($element_modules, $modules);
      }

      foreach (array_intersect(array_unique($modules), module_list()) as $module) {
        if ($differences = array_filter(features_override_module_component_overrides($module, $component))) {
          $cache[$component] = isset($cache[$component]) ? array_merge($differences, $cache[$component]) : $differences;
        }
      }
      $cache[$component] = isset($cache[$component]) ? array_filter(array_unique($cache[$component])) : array();
    }
  }

  if ($component_key && $element_key) {
    return !empty($cache[$component_key][$element_key]) ? $cache[$component_key][$element_key] : array();
  }
  elseif ($component_key) {
    return !empty($cache[$component_key]) ? $cache[$component_key] : array();
  }
  return $cache;
}


/**
 * Get overrides for specific module/component
 */
function features_override_module_component_overrides($module, $component) {
  static $cache = array();

  if (isset($cache[$module][$component])) {
    return $cache[$module][$component];
  }

  $normal = features_get_normal($component, $module);
  $default = features_get_default($component, $module, FALSE, TRUE);
  // Can't use _features_sanitize as that resets some keys.
  _features_override_sanitize($normal);
  _features_override_sanitize($default);
  $component_overrides = array();
  $ignore_keys = module_invoke_all('features_override_ignore', $component);
  if ($normal && is_array($normal) || is_object($normal)) {
    foreach ($normal as $name => $properties) {
      $component_overrides[$name] = array(
        'additions' => array(), 
        'deletions' => array(),
      );
      if (isset($default[$name])) {
        _features_override_set_additions($default[$name], $properties, $component_overrides[$name]['additions'], $ignore_keys);
        _features_override_leave_hive($default[$name]);
        _features_override_set_deletions($default[$name], $properties, $component_overrides[$name]['deletions'], $ignore_keys);
        _features_override_leave_hive($default[$name]);
      }
      if (!array_filter($component_overrides[$name])) {
        $component_overrides[$name] = FALSE;
      }
    }
  }

  $cache[$module][$component] = $component_overrides;
  return $component_overrides;
}

/**
 * "Sanitizes" an array recursively, performing:
 * - Sort an array by its keys (assoc) or values (non-assoc).
 */
function _features_override_sanitize(&$array) {
  if (is_array($array)) {
    $is_assoc = (array_keys($array) !== range(0, count($array) - 1));
    if ($is_assoc) {
      ksort($array);
    }
    else {
      sort($array);
    }
    foreach ($array as $k => $v) {
      if (is_array($v)) {
        _features_override_sanitize($array[$k]);
      }
    }
  }
}


/**
 * Add a variable to the hive of arrays and objects which
 * are tracked for whether they have recursive entries
 *
 * @param &$bee
 *   Array or object.
 * @return
 *   Array all the bees.
 */
function _features_override_hive(&$bee = NULL) {
  static $bees = array();

  // New bee ?
  if (!is_null($bee) && (!is_array($bee) || count($bee) > 0)) {
    // Stain it.
    $recursion_marker = FEATURES_OVERRIDE_RECURSION_MARKER;
    (is_object($bee))
      ? @($bee->$recursion_marker++)
      : @($bee[$recursion_marker]++);

    $bees[0][] =& $bee;
  }

  // Return all bees.
  return $bees[0];
}

/**
 * Remove markers from previously marked elements.
 */
function _features_override_leave_hive() {
  $hive = _features_override_hive();
  foreach ($hive as $i => $bee) {
    $recursion_marker = FEATURES_OVERRIDE_RECURSION_MARKER;
    if (is_object($bee)) {
      unset($hive[$i]->$recursion_marker);
    }
    else {
      unset($hive[$i][$recursion_marker]);
    }
  }
}

/**
 * Return a marker of recursion.
 */
function _features_override_recursion_marker(&$normal, $object) {
  _features_override_hive($normal);
  // Test for references in order to prevent endless recursion loops.
  $recursion_marker = FEATURES_OVERRIDE_RECURSION_MARKER;
  $r = (int) ($object ? @$normal->$recursion_marker : @$normal[$recursion_marker]);
  return $r;
}

/**
 * Helper function to set the additions and alters between default and normal components.
 */
function _features_override_set_additions($default, $normal, &$additions, $ignore_keys = array(), $level = 0, $keys = array()) {
  $object = is_object($normal);

  // Recursion detected.
  if (_features_override_recursion_marker($default, $object) > 1) {
    return;
  }

  foreach ($normal as $key => $value) {
    // Don't register extra fields as an override.
    // The 'content_has_extra_fields' flag is added to indicate that there are
    // added fields. However, these should simply be added to a feature as 
    // fields; they are not an override.
    // Ignore the marker used to track detect recursion.
    if (isset($ignore_keys[$key]) && ($level == $ignore_keys[$key])) {
      continue;
    }
    if (!is_numeric($key) && in_array($key, array('content_has_extra_fields', FEATURES_OVERRIDE_RECURSION_MARKER))) {
      return;
    }
    if ($object) {
      if (!property_exists($default, $key) || (is_scalar($value) && ($default->$key !== $value))) {
        $keys = array_merge($keys, array(array('type' => 'object', 'key' => $key)));
        $additions[features_override_make_key($keys)] = array(
          'keys' => $keys,
          'value' => $value,
        );
      }
      elseif (property_exists($default, $key) && ($default->$key !== $value)) {
        _features_override_set_additions($default->$key, $value, $additions, $ignore_keys, $level+1, array_merge($keys, array(array('type' => 'object', 'key' => $key))));
      }
    }
    elseif (is_array($normal)) {
      if (!array_key_exists($key, $default) || (is_scalar($value) && ($default[$key] !== $value))) {
        $keys = array_merge($keys, array(array('type' => 'array', 'key' => $key)));
        $additions[features_override_make_key($keys)] = array(
          'keys' => $keys,
          'value' => $value,
        );
      }
      elseif (array_key_exists($key, $default) && (!is_null($value) && ($default[$key] !== $value))) {
        _features_override_set_additions($default[$key], $value, $additions, $ignore_keys, $level+1, array_merge($keys, array(array('type' => 'array', 'key' => $key))));
      }
    }
  }
}

/**
 * Helper function to set the deletions between default and normal features.
 */
function _features_override_set_deletions($default, $normal, &$deletions, $ignore_keys = array(), $level = 0, $keys = array()) {
  $object = is_object($default);

  // Recursion detected.
  if (_features_override_recursion_marker($default, $object) > 1) {
    return;
  }

  foreach ($default as $key => $value) {
    if (isset($ignore_keys[$key]) && ($level == $ignore_keys[$key])) {
      continue;
    }
    // Ignore the marker used to track detect recursion.
    if ($key == FEATURES_OVERRIDE_RECURSION_MARKER) {
      continue;
    }
    if ($object) {
      if (!property_exists($normal, $key)) {
        $_keys = array_merge($keys, array(array('type' => 'object', 'key' => $key)));
        $deletions[features_override_make_key($_keys)] = array(
          'keys' => $_keys,
        );
      }
      elseif (property_exists($normal, $key) && (is_array($value) || is_object($value))) {
        _features_override_set_deletions($value, $normal->$key, $deletions, $ignore_keys, $level+1, array_merge($keys, array(array('type' => 'object', 'key' => $key))));
      }
    }
    else {
      if (!array_key_exists($key, $normal)) {
        $_keys = array_merge($keys, array(array('type' => 'array', 'key' => $key)));
        $deletions[features_override_make_key($_keys)] = array(
          'keys' => $_keys,
        );
      }
      elseif (array_key_exists($key, $normal) && (is_array($value) || is_object($value))) {
        _features_override_set_deletions($value, $normal[$key], $deletions, $ignore_keys, $level+1, array_merge($keys, array(array('type' => 'array', 'key' => $key))));
      }
    }
  }
}

/**
 * Exports an array that describes nesting of keys.
 */
function features_override_export_keys($keys) {
  $line = '';
  foreach ($keys as $key) {
    if ($key['type'] == 'object') {
      $line .= "->{$key['key']}";
    }
    else {
      $line .= "['{$key['key']}']";
    }
  }
  return $line;
}

