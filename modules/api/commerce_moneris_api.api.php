<?php

/**
 * @file
 * This file contains no working PHP code; it exists to provide additional
 * documentation for doxygen as well as to document hooks in the standard
 * Drupal manner.
 */

/**
 * Allows modules to alter transaction details before sending them to Moneris.
 *
 * @param array $txn_array
 *   An array of elements being sent to Moneris API payment gateway.
 * @param object $order
 *   An order object being paid for.
 */
function hook_commerce_moneris_api_txnarray_alter(&$txn_array, $order) {
  $txn_array['commcard_invoice'] = 'Invoice #' . $order->order_number;
}
