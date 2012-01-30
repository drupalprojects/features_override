<?php

/**
 * Renders the differences for a component and element
 */
function features_override_render_differences($component, $element) {
  module_load_include('export.inc', 'features_override');
  module_load_include('export.inc', 'features');
  if ($differences = features_override_get_overrides($component, $element)) {
    if (!empty($_GET['key'])) {
      if (!empty($differences['additions'][$_GET['key']])) {
        return '<pre>' .  implode("\n", features_override_features_export_render_addition($differences['additions'][$_GET['key']], $element)) . '</pre>';
      }
      elseif (!empty($differences['deletions'][$_GET['key']])) {
        return '<pre>' .  implode("\n", features_override_features_export_render_deletion($differences['deletions'][$_GET['key']], $element)) . '</pre>';
      }
      else {
        return t('Unable to find referenced information.');
      }
    }
    else {
      $code = array();
      if (!empty($differences['additions'])) {
        foreach ($differences['additions'] as $alter) {
          $code = array_merge(features_override_features_export_render_addition($alter, $element), $code);
        }
      }
      if (!empty($differences['deletions'])) {
        foreach ($differences['deletions'] as $alter) {
          $code = array_merge(features_override_features_export_render_deletion($alter, $element), $code);
        }
      }
      return '<pre>' . implode("\n", $code) . '</pre>';
    }
  }
  else {
    return t('No overrides were found.');
  }
}
