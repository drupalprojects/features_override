<?php

/**
 * Alter default features.
 */
function features_override_features_default_alter(&$items, $component) {
  $overrides = features_override_load_all();
  foreach ($overrides as $override) {
    if (isset($items[$override->component_id]) && $override->component_type == $component && !empty($override->value)) {
      if (isset ($override->value['additions']) && !empty($override->value['additions'])) {
        foreach ($override->value['additions'] as $addition) {
          _features_override_set_item($items[$override->component_id], 'add', $addition['keys'], $addition['value'], $addition['order']);
        }
      }
      if (isset ($override->value['deletions']) && !empty($override->value['deletions'])) {
        foreach ($override->value['deletions'] as $deletion) {
          _features_override_set_item($items[$override->component_id], 'delete', $deletion['keys']);
        }
      }
    }
  }
}

/**
 * Set a single addition or deletion for a component.
 */
function _features_override_set_item(&$item, $action, $keys, $value = NULL, $order = NULL) {
  $key = array_shift($keys);
  if ($key['type'] == 'object') {
    if (count($keys) && isset($item->{$key['key']})) {
      _features_override_set_item($item->{$key['key']}, $action, $keys, $value, $order);
    }
    else {
      switch ($action) {
        case 'add':
          $item->{$key['key']} = $value;
          if (isset($order) && (array_keys((array)$item) !== range(0, count((array)$item) - 1))) {
            $compare = create_function('$a,$b', '$order = ' . var_export($order, TRUE) . '; $apos = array_search($a, $order); $bpos = array_search($b, $order); if ($apos == $bpos) return 0; return ($apos < $bpos) ? -1 : 1;');
            $item = (array)$item;
            uksort($item, $compare);
            $item = (object)$item;
          }
          break;
        case 'delete':
          unset($item->{$key['key']});
          break;
      }
    }
  }
  else {
    if (count($keys) && isset($item[$key['key']])) {
      _features_override_set_item($item[$key['key']], $action, $keys, $value, $order);
    }
    else {
      switch ($action) {
        case 'add':
          $item[$key['key']] = $value;
          if (isset($order) && (array_keys($item) !== range(0, count($item) - 1))) {
            $compare = create_function('$a,$b', '$order = ' . var_export($order, TRUE) . '; $apos = array_search($a, $order); $bpos = array_search($b, $order); if ($apos == $bpos) return 0; return ($apos < $bpos) ? -1 : 1;');
            uksort($item, $compare);
          }
          break;
        case 'delete':
          unset($item[$key['key']]);
          break;
      }
    }
  }
}

