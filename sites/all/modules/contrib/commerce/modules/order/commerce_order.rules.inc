<?php

/**
 * @file
 * Rules integration for orders.
 *
 * @addtogroup rules
 * @{
 */

/**
 * Implements hook_rules_event_info().
 */
function commerce_order_rules_event_info() {
  $events = array();

  $events['commerce_order_presave'] = array(
    'label' => t('Before saving an order'),
    'group' => t('Commerce Order'),
    'variables' => commerce_order_rules_event_variables(t('Order', array(), array('context' => 'a drupal commerce order')), TRUE, TRUE),
    'access callback' => 'commerce_order_rules_access',
  );
  $events['commerce_order_insert'] = array(
    'label' => t('After saving a new order'),
    'group' => t('Commerce Order'),
    'variables' => commerce_order_rules_event_variables(t('Created order'), TRUE),
    'access callback' => 'commerce_order_rules_access',
  );
  $events['commerce_order_update'] = array(
    'label' => t('After updating an existing order'),
    'group' => t('Commerce Order'),
    'variables' => commerce_order_rules_event_variables(t('Updated order'), TRUE),
    'access callback' => 'commerce_order_rules_access',
  );
  $events['commerce_order_delete'] = array(
    'label' => t('After deleting an order'),
    'group' => t('Commerce Order'),
    'variables' => commerce_order_rules_event_variables(t('Deleted order')),
    'access callback' => 'commerce_order_rules_access',
  );

  return $events;
}

/**
 * Returns a variables array for order events.
 *
 * @param $label
 *   The label for the primary order variable.
 * @param $unchanged
 *   Boolean indicating whether or not to include the unchanged order.
 * @param $skip_save
 *   Boolean indicating whether or not the primary order variable should skip
 *     saving after event execution.
 */
function commerce_order_rules_event_variables($label, $unchanged = FALSE, $skip_save = FALSE) {
  $variables = array(
    'order' => array(
      'type' => 'commerce_order',
      'label' => $label,
      'skip save' => $skip_save,
    ),
  );

  if ($unchanged) {
    $variables['order_unchanged'] = array(
      'type' => 'commerce_order',
      'label' => t('Unchanged order'),
      'skip save' => TRUE,
      'handler' => 'rules_events_entity_unchanged',
    );
  }

  return $variables;
}

/**
 * Rules integration access callback.
 */
function commerce_order_rules_access($type, $name) {
  if ($type == 'event' || $type == 'condition') {
    return commerce_order_access('view');
  }
}

/**
 * Implements hook_rules_condition_info().
 */
function commerce_order_rules_condition_info() {
  $conditions = array();

  $conditions['commerce_order_compare_address'] = array(
    'label' => t('Order address component comparison'),
    'parameter' => array(
      'order' => array(
        'type' => 'commerce_order',
        'label' => t('Order'),
        'description' => t('The order containing the profile reference with the address in question. In most cases, you will use line-item:order.'),
      ),
      'address_field' => array(
        'type' => 'text',
        'label' => t('Address'),
        'options list' => 'commerce_order_address_field_options_list',
        'description' => t('The address associated with this order whose component you want to compare.'),
        'restriction' => 'input',
      ),
      'address_component' => array(
        'type' => 'text',
        'label' => t('Address component'),
        'options list' => 'commerce_order_address_component_options_list',
        'description' => t('The actual address component you want to compare. Common names of address components are given in parentheses.'),
        'restriction' => 'input',
      ),
      'operator' => array(
        'type' => 'text',
        'label' => t('Operator'),
        'description' => t('The comparison operator.'),
        'optional' => TRUE,
        'default value' => 'equals',
        'options list' => 'commerce_order_address_comparison_operator_options_list',
        'restriction' => 'input',
      ),
      'value' => array(
        'type' => 'text',
        'label' => t('Value'),
        'description' => t('The value to compare against the address component. Bear in mind that addresses using select lists for various components may use a value different from the option you select. For example, countries are selected by name, but the value is the two letter abbreviation.'),
      ),
    ),
    'group' => t('Commerce Order'),
    'callbacks' => array(
      'execute' => 'commerce_order_rules_compare_address',
    ),
  );

  return $conditions;
}

/**
 * Options list callback: address fields for the address comparison condition.
 */
function commerce_order_address_field_options_list() {
  $options = array();

  // Retrieve a list of all address fields on customer profile bundles.
  $address_fields = commerce_info_fields('addressfield', 'commerce_customer_profile');

  // Loop over every customer profile reference field on orders.
  foreach (commerce_info_fields('commerce_customer_profile_reference', 'commerce_order') as $field_name => $field) {
    // Retrieve the type of customer profile referenced by this field.
    $type = $field['settings']['profile_type'];

    // Loop over every address field looking for any attached to this bundle.
    foreach ($address_fields as $address_field_name => $address_field) {
      if (in_array($type, $address_field['bundles']['commerce_customer_profile'])) {
        // Add it to the options list.
        $instance = field_info_instance('commerce_customer_profile', 'commerce_customer_address', $type);

        $options[commerce_customer_profile_type_get_name($type)][$field_name . '|' . $address_field_name] = check_plain($instance['label']);
      }
    }
  }

  if (empty($options)) {
    drupal_set_message(t('No order addresses could be found for comparison.'), 'error');
  }

  return $options;
}

/**
 * Options list callback: components for the address comparison condition.
 */
function commerce_order_address_component_options_list() {
  return array(
    'country' => t('Country'),
    'name_line' => t('Full name'),
    'first_name' => t('First name'),
    'last_name' => t('Last name'),
    'organisation_name' => t('Company name'),
    'thoroughfare' => t('Thoroughfare (Street address)'),
    'premise' => t('Premise (Building)'),
    'sub_premise' => t('Sub-premise (Suite)'),
    'locality' => t('Locality (City)'),
    'dependent_locality' => t('Dependent locality (Town)'),
    'administrative_area' => t('Administrative area (State / Province)'),
    'sub_administrative_area' => t('Sub-administrative area (District)'),
    'postal_code' => t('Postal code'),
  );
}

/**
 * Options list callback: operators for the address comparison condition.
 */
function commerce_order_address_comparison_operator_options_list() {
  return array(
    'equals' => t('equals'),
    'begins with' => t('begins with'),
    'contains' => t('contains'),
  );
}

/**
 * Condition callback: compares an address component against the given value.
 */
function commerce_order_rules_compare_address($order, $address_field, $component, $operator, $value) {
  list($field_name, $address_field_name) = explode('|', $address_field);

  // If we actually received a valid order...
  if (!empty($order)) {
    $wrapper = entity_metadata_wrapper('commerce_order', $order);

    // And if we can actually find the requested address data...
    if (!empty($wrapper->{$field_name}) && !empty($wrapper->{$field_name}->{$address_field_name})) {
      $address = $wrapper->{$field_name}->{$address_field_name}->value();

      // Make a comparison based on the operator.
      switch ($operator) {
        case 'equals':
          return $address[$component] == $value;
        case 'begins with':
          return strpos($address[$component], $value) === 0;
        case 'contains':
          return strpos($address[$component], $value) !== FALSE;
      }
    }
  }

  return FALSE;
}

/**
 * Implements hook_rules_action_info().
 */
function commerce_order_rules_action_info() {
  $actions = array();

  $actions['commerce_order_update_state'] = array(
    'label' => t('Update the order state'),
    'parameter' => array(
      'order' => array(
        'type' => 'commerce_order',
        'label' => t('Order to update'),
      ),
      'order_state' => array(
        'type' => 'text',
        'label' => t('Order state'),
        'description' => t('Select the order state whose default status the order will be updated to.'),
        'options list' => 'commerce_order_state_options_list',
      ),
    ),
    'group' => t('Commerce Order'),
    'callbacks' => array(
      'execute' => 'commerce_order_rules_update_state',
    ),
  );

  $actions['commerce_order_update_status'] = array(
    'label' => t('Update the order status'),
    'parameter' => array(
      'order' => array(
        'type' => 'commerce_order',
        'label' => t('Order to update'),
      ),
      'order_status' => array(
        'type' => 'text',
        'label' => t('Order status'),
        'options list' => 'commerce_order_status_options_list',
      ),
    ),
    'group' => t('Commerce Order'),
    'callbacks' => array(
      'execute' => 'commerce_order_rules_update_status',
    ),
  );

  return $actions;
}

/**
 * Rules action: updates an order's status to the default status of the given
 *   order state.
 */
function commerce_order_rules_update_state($order, $name) {
  $order_state = commerce_order_state_load($name);
  commerce_order_status_update($order, $order_state['default_status'], FALSE, TRUE, t('Order state updated via Rules.'));
}

/**
 * Rules action: updates an order's status using the Order API.
 */
function commerce_order_rules_update_status($order, $name) {
  commerce_order_status_update($order, $name, FALSE, TRUE, t('Order status updated via Rules.'));
}

/**
 * @}
 */