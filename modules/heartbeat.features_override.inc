<?php

/**
 * Implementation of hook_heartbeat_stream_info_alter().
 */
function features_override_heartbeat_stream_info_alter(&$items) {
  features_override_features_default_alter($items, "heartbeat_streams");
}

/**
 * Implementation of hook_heartbeat_template_info_alter().
 */
function features_override_heartbeat_template_info_alter(&$items) {
  features_override_features_default_alter($items, "heartbeat_messages");
}
