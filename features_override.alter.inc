<?php
// $Id$

/**
 * Alter default features.
 */
function features_override_features_default_alter(&$items, $component) {
  $overrides = features_override_load_all();
  foreach ($overrides as $override) {
    if (isset($items[$override->component_id]) && $override->component_type == $component && !empty($override->value)) {
      if (!empty($override->value['additions'])) {
        foreach ($override->value['additions'] as $addition) {
          $item =& $items[$override->component_id];
          foreach ($addition['keys'] as $key) {
            if ($key['type'] == 'object') {
              $item =& $item->{$key['key']};
            }
            else {
              $item =& $item[$key['key']];
            }
          }
          $item = $addition['value'];
        }
      }
      if (!empty($override->value['deletions'])) {
        foreach ($override->value['deletions'] as $deletion) {
          $item =& $items[$override->component_id];
          $last_key = array_pop($deletion['keys']);
          foreach ($deletion['keys'] as $key) {
            if ($key['type'] == 'object') {
              $item =& $item->{$key['key']};
            }
            else {
              $item =& $item[$key['key']];
            }
          }
          $copy = is_object($item) ? clone($item) : $item;
          if ($last_key['type'] == 'object') {
            unset($copy->{$last_key['key']});
          }
          else {
            unset($copy[$last_key['key']]);
          }
          $item = $copy;
        }
      }
    }
  }
}

/**
 * Adapted from features.ctools.inc; this is a wild hack, but effective.
 *
 * Dynamically declare alter functions for all supported components.
 *
 * Because this function indirectly calls features_include() on every page, it has
 * significant performance implications.
 *
 * @todo
 *   Either introduce a general alter hook for all features components or
 *   determine how to limit these alter calls to pages where they are needed.
 */
foreach (features_override_supported_components(TRUE) as $component => $default_hook) {
  $code = 'function features_override_'. $default_hook .'_alter(&$items) { features_override_features_default_alter($items, "' . $component .'"); }';
  eval($code);
}

