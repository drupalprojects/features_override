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
  $options = array();

  $overrides = features_override_get_overrides();
  foreach ($overrides as $component => $elements) {
    foreach ($elements as $key => $element) {
      $identifier = "{$component}.{$key}";
      $identifier_spaced = "{$component} {$key} ";
      $options[$identifier] = $identifier_spaced . ' - all changes ';
      foreach ($element['additions'] as $change_key => $changes) {
        $options[$identifier . '.' . $change_key] = " - " . $identifier_spaced . ' addition: of ' . features_override_export_keys($changes['keys']);
      }
      foreach ($element['deletions'] as $change_key => $changes) {
        $options[$identifier . '.' . $change_key] = ' - ' . $identifier_spaced . ' deletion of ' . features_override_export_keys($changes['keys']);
      }
    }
  }

  return $options;
}

/**
 * Implements hook_features_export().
 */
function features_override_features_export($data, &$export, $module_name = '') {
  $map = features_get_default_map('features_override');

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
      $export['features']['features_override'][$identifier] = $identifier;
    }
  }

}

/**
 * Implements hook_features_export_render().
 */
function features_override_features_export_render($module, $data, $export = NULL) {
  $list = $overrides_to_export = array();

  // Go through all data collecting the items exports to create.
  foreach ($data as $identifier) {
    // Something is adding extra quote marks.
    list($component, $element, $keys) = features_override_parse_identifier($identifier);


    $component_code[$component] = isset($component_code[$component]) ? $component_code[$component] : array();
    $overrides_to_export[$component] = isset($overrides_to_export[$component]) ? $overrides_to_export[$component] : array();
    $overrides = features_override_get_overrides($component);

    // The entire component is being exported
    if (empty($keys) && isset($overrides[$element])) {
      $overrides_to_export[$component][$element] = $overrides[$element];
    }
    // Just specific keys are being exported so add them if set.
    elseif (!empty($keys) && !empty($overrides[$element])) {
      if (isset($overrides[$element]['additions'][$keys])) {
        $overrides_to_export[$component][$element]['additions'][] = $overrides[$element]['additions'][$keys];
      }
      elseif (isset($overrides[$element]['deletions'][$keys])) {
        $overrides_to_export[$component][$element]['deletions'][] = $overrides[$element]['deletions'][$keys];
      }
    }
  }


  $features_override_code = array();
  $component_code = array();
  foreach ($overrides_to_export as $component => $elements) {
    foreach ($elements as $element => $overrides) {
      $component_code[$component] = isset($component_code[$component]) ? $component_code[$component] : array();
      $component_start = "\$data['$element']";
      if (!empty($overrides['additions'])) {
        foreach ($overrides['additions'] as $alter) {
          $code_line = $component_start . features_override_export_keys($alter['keys']);
          $test_line = $component_start . features_override_export_keys(array_slice($alter['keys'], 0, count($alter['keys']) - 1));
          $value_export = features_var_export($alter['value']);
          $component_code[$component][] = "  if (!empty(" . $test_line . ")) {";
          $component_code[$component][] = "    " . $code_line . ' = ' . $value_export . ';';
          $component_code[$component][] = "  }";
        }
      }
      if (!empty($overrides['deletions'])) {
        foreach ($overrides['deletions'] as $alter) {
          $code_line = features_override_export_keys($alter['keys']);
          $value_export = features_var_export($alter['value']);
          $component_code[$component][] =  '  unset(' . $component_start . $code_line . ');';
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


