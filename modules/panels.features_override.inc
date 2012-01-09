<?php

/**
 * Implements hook_default_panels_renderer_pipeline_alter().
 */
function features_override_default_panels_renderer_pipeline_alter(&$items) {
  features_override_features_default_alter($items, "panels_renderer_pipeline");
}

/**
 * Implements hook_default_panels_layout_alter().
 */
function features_override_default_panels_layout_alter(&$items) {
  features_override_features_default_alter($items, "panels_layout");
}
