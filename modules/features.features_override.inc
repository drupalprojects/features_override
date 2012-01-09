<?php

/**
 * Implements hook_default_wysiwyg_profile_alter().
 */
function features_override_default_wysiwyg_profile_alter(&$items) {
  features_override_features_default_alter($items, "wysiwyg_profile");
}

/**
 * Implements hook_field_default_fields_alter().
 */
function features_override_field_default_fields_alter(&$items) {
  features_override_features_default_alter($items, "field");
}

/**
 * Implements hook_filter_default_formats_alter().
 */
function features_override_filter_default_formats_alter(&$items) {
  features_override_features_default_alter($items, "filter");
}

/**
 * Implements hook_image_default_styles_alter().
 */
function features_override_image_default_styles_alter(&$items) {
  features_override_features_default_alter($items, "image");
}

/**
 * Implements hook_menu_default_menu_custom_alter().
 */
function features_override_menu_default_menu_custom_alter(&$items) {
  features_override_features_default_alter($items, "menu_custom");
}

/**
 * Implements hook_menu_default_menu_links_alter().
 */
function features_override_menu_default_menu_links_alter(&$items) {
  features_override_features_default_alter($items, "menu_links");
}

/**
 * Implements hook_menu_default_items_alter().
 */
function features_override_menu_default_items_alter(&$items) {
  features_override_features_default_alter($items, "menu");
}

/**
 * Implements hook_taxonomy_default_vocabularies_alter().
 */
function features_override_taxonomy_default_vocabularies_alter(&$items) {
  features_override_features_default_alter($items, "taxonomy");
}

/**
 * Implements hook_user_default_roles_alter().
 */
function features_override_user_default_roles_alter(&$items) {
  features_override_features_default_alter($items, "user_role");
}

/**
 * Implements hook_user_default_permissions_alter().
 */
function features_override_user_default_permissions_alter(&$items) {
  features_override_features_default_alter($items, "user_permission");
}

/**
 * Implements hook_default_search_api_server_alter().
 */
function features_override_default_search_api_server_alter(&$items) {
  features_override_features_default_alter($items, "search_api_server");
}

/**
 * Implements hook_default_search_api_index_alter().
 */
function features_override_default_search_api_index_alter(&$items) {
  features_override_features_default_alter($items, "search_api_index");
}

/**
 * Implements hook_default_search_api_facet_alter().
 */
function features_override_default_search_api_facet_alter(&$items) {
  features_override_features_default_alter($items, "search_api_facet");
}

/**
 * Implements hook_flag_default_flags_alter().
 */
function features_override_flag_default_flags_alter(&$items) {
  features_override_features_default_alter($items, "flag");
}

/**
 * Implements hook_commerce_product_default_types_alter().
 */
function features_override_commerce_product_default_types_alter(&$items) {
  features_override_features_default_alter($items, "commerce_product_type");
}

/**
 * Implements hook_default_og_membership_type_alter().
 */
function features_override_default_og_membership_type_alter(&$items) {
  features_override_features_default_alter($items, "og_membership_type");
}

/**
 * Implements hook_default_profile2_type_alter().
 */
function features_override_default_profile2_type_alter(&$items) {
  features_override_features_default_alter($items, "profile2_type");
}
