<?php

/**
 * @file
 * Implements feature specific hooks
 */

module_load_include('export.inc', 'features_override');

/**
 * Implements hook_features_export_options().
 */
function features_override_features_export_options() {
  $options = $links = array();
  drupal_add_js(drupal_get_path('module', 'features_override') . '/features_override_form.js');

  $overrides = features_override_get_overrides();
  foreach ($overrides as $component => $elements) {
    foreach ($elements as $key => $element) {
      $identifier = "{$component}.{$key}";
      $identifier_spaced = "{$component} {$key} ";
      $options[$identifier] = $identifier_spaced . ' - all changes ';
      $links[str_replace('.', '__44__', $identifier)] = url('admin/structure/features/override/' . $component . '/' . $key);
      foreach ($element['additions'] as $change_key => $changes) {
        $options[$identifier . '.' . $change_key] = " - " . $identifier_spaced . ' addition: of ' . features_override_export_keys($changes['keys']);
        $links[str_replace('.', '__44__', $identifier . '.' . $change_key)] = url('admin/structure/features/override/' . $component . '/' . $key, array('query' => array('key' => $change_key)));
      }
      foreach ($element['deletions'] as $change_key => $changes) {
        $options[$identifier . '.' . $change_key] = ' - ' . $identifier_spaced . ' deletion of ' . features_override_export_keys($changes['keys']);
        $links[str_replace('.', '__44__', $identifier . '.' . $change_key)] = url('admin/structure/features/override/' . $component . '/' . $key, array('query' => array('key' => $change_key)));
      }
    }
  }
  drupal_add_js(array('features_override_links' => $links), 'setting');

  return $options;
}

/**
 * Implements hook_features_export().
 */
function features_override_features_export($data, &$export, $module_name = '') {
  $pipe = array('features_override' => array());
  $map = features_get_default_map('features_override');
  static $options;

  foreach ($data as $identifier) {
    // If this override bundle is already provided by another module, remove the field
    // and add the other module as a dependency.
    if (isset($map[$identifier]) && $map[$identifier] != $module_name) {
      if (isset($export['features']['features_override'][$identifier])) {
        unset($export['features']['features_override'][$identifier]);
      }
      $module = $map[$identifier];
      $export['dependencies'][$module] = $module;
    }
    // If the field has not yet been exported, add it
    else {
      list($component, $element, $keys) = features_override_parse_identifier($identifier);
      // If this is an 'all' key, see if anyone has exported any keys for this.
      $convert = FALSE;
      if (empty($keys)) {
        foreach ($map as $identifier_key => $modle) {
          if ($identifier != $identifier_key && strpos($identifier_key, $identifier . '.') === 0 && !empty($map[$identifier_key])) {
            $convert = TRUE;
            break;
          }
        }
      }

      // At least one indivudal identifier is exported, so convert this
      // to indivudal components.
      if ($convert) {
        if (!isset($options)) {
          $options = features_override_features_export_options();
        }
        foreach ($options as $option_key => $option_name) {
          list($options_component, $options_element, $options_keys) = features_override_parse_identifier($option_key);
          if ($options_element == $element && empty($map[$option_key]) && empty($data[$option_key])) {
            $pipe['features_override'][] = $option_key;
          }
        }
      }
      else {
        $export['features']['features_override'][$identifier] = $identifier;
        // Add dependency to defining module
        $component_map = features_get_default_map($component);
        if (!empty($component_map[$element])) {
          $export['dependencies'][$component_map[$element]] = $component_map[$element];
        }
      }
    }
  }
  return $pipe;
}

/**
 * Implements hook_features_export_render().
 */
function features_override_features_export_render($module, $data, $export = NULL) {
  $list = $overrides_to_export = array();


  // Go through all data collecting the items exports to create.
  $features_override_code = array(
    '  // This code is only used for informing features UI.',
    '  $overrides = array();',
  );
  foreach ($data as $identifier) {
    // Something is adding extra quote marks.
    list($component, $element, $keys) = features_override_parse_identifier($identifier);


    $component_code[$component] = isset($component_code[$component]) ? $component_code[$component] : array();
    $overrides_to_export[$component] = isset($overrides_to_export[$component]) ? $overrides_to_export[$component] : array();
    $overrides = features_override_get_overrides($component);

    // The entire component is being exported
    if (empty($keys) && isset($overrides[$element])) {
      $overrides_to_export[$component][$element] = $features_override_value = $overrides[$element];
    }
    // Just specific keys are being exported so add them if set.
    elseif (!empty($keys) && !empty($overrides[$element])) {
      $features_override_value = array(
        'additions' => array(),
        'deletions' => array()
      );

      if (isset($overrides[$element]['additions'][$keys])) {
        $overrides_to_export[$component][$element]['additions'][] = $overrides[$element]['additions'][$keys];
        $features_override_value['additions'][] = $overrides[$element]['additions'][$keys];
      }
      elseif (isset($overrides[$element]['deletions'][$keys])) {
        $overrides_to_export[$component][$element]['deletions'][] = $overrides[$element]['deletions'][$keys];
        $features_override_value['deletions'][] = $overrides[$element]['deletions'][$keys];
      }
      $features_override_code[] = '  $overrides["' . $identifier  . '"] = ' . features_var_export($features_override_value, '  ') . ';';
    }
  }

  // Create the default hook that defines all exported overrides.
  $features_override_code[] = '';
  $features_override_code[] = ' return $overrides;';
  $list[FEATURES_OVERRIDE_HOOK] = implode("\n", $features_override_code);

  $component_code = array();
  foreach ($overrides_to_export as $component => $elements) {
    foreach ($elements as $element => $overrides) {
      $component_code[$component] = isset($component_code[$component]) ? $component_code[$component] : array();
      if (!empty($overrides['additions'])) {
        foreach ($overrides['additions'] as $alter) {
          $component_code[$component] = array_merge(features_override_features_export_render_addition($alter, $element), $component_code[$component]);
        }
      }
      if (!empty($overrides['deletions'])) {
        foreach ($overrides['deletions'] as $alter) {
          $component_code[$component] = array_merge(features_override_features_export_render_deletion($alter, $element), $component_code[$component]);
        }
      }
    }
  }

  $info = features_get_components();
  foreach($component_code as $component => $code) {
    $list[$info[$component]['default_hook'] . '_alter'] = array(
      'code' => implode("\n", $code),
      'args' => '&$data',
    );
  }

  return $list;
}

/**
 * Renders the addition/change to an element.
 */
function features_override_features_export_render_addition($alter, $element) {
  $code = array();
  $component_start = "\$data['$element']";
  $code_line = features_override_export_keys($alter['keys']);
  $test_line = features_override_export_keys(array_slice($alter['keys'], 0, count($alter['keys']) - 1));
  $value_export = features_var_export($alter['value']);
  $code[] = "  if (!empty(" . $component_start . $test_line . ")) {";
  $code[] = "    " . $component_start . $code_line . ' = ' . $value_export . ';';
  $code[] = "  }";
  return $code;
}

/**
 * Renders the deletion to an element.
 */
function features_override_features_export_render_deletion($alter, $element) {
  $code = array();
  $component_start = "\$data['$element']";
  $code_line = features_override_export_keys($alter['keys']);
  $test_line = features_override_export_keys(array_slice($alter['keys'], 0, count($alter['keys']) - 1));
  $code[] = "  if (!empty(" . $component_start . $test_line . ")) {";
  $code[] =  '    unset(' . $component_start . $code_line . ');';
  $code[] = "  }";
  return $code;
}

/**
 * Implements hook_features_revert().
 */
function features_override_features_revert($module) {
  features_override_features_rebuild($module);
}

/**
 * Implements of hook_features_rebuild().
 * Rebuilds fields from code defaults.
 */
function features_override_features_rebuild($module) {
  // rebuild parent modules we are overriding

  // first build list of components we are overriding
  $parents = array();
  $data = features_get_component_map();
  foreach ($data['features_override'] as $identifier => $component_modules) {
    list($component, $element, $keys) = features_override_parse_identifier($identifier);
    if (isset($data[$component][$element])) {
      foreach ($data[$component][$element] as $module) {
        $parents[$module][$component] = $component;
      }
    }
  }
  // first, update get_default cache so rebuild will work on latest data
  foreach ($parents as $module => $element) {
    foreach ($element as $component) {
      features_get_default($component, $module, TRUE, TRUE);
    }
  }
  // rebuild those parent components
  features_rebuild($parents);
}


