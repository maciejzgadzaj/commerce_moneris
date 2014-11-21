<?php

/**
 * @file
 * Administrative forms for the Moneris API module.
 */

/**
 * Form callback: allows the user to refund a transaction.
 */
function commerce_moneris_api_refund_form($form, &$form_state, $order, $transaction) {
  $form_state['order'] = $order;
  $form_state['transaction'] = $transaction;

  // Load and store the payment method instance for this transaction.
  $payment_method = commerce_payment_method_instance_load($transaction->instance_id);
  $form_state['payment_method'] = $payment_method;

  // We need the max amount that still could be refunded.
  $wrapper = entity_metadata_wrapper('commerce_order', $order);
  $order_total = $wrapper->commerce_order_total->value();
  $order_balance = commerce_payment_order_balance($order);
  $balance_amount = $order_total['amount'] - $order_balance['amount'];

  $description = implode('<br />', array(
    t('Captured: @amount', array('@amount' => commerce_currency_format($transaction->amount, $transaction->currency_code))),
    t('Current order balance: @amount', array('@amount' => commerce_currency_format($balance_amount, $order_balance['currency_code']))),
  ));

  // Convert the price amount to a user friendly decimal value.
  $balance_amount = number_format(commerce_currency_amount_to_decimal($balance_amount, $order_balance['currency_code']), 2, '.', '');
  $form_state['max_refund_amount'] = $balance_amount;

  $form['amount'] = array(
    '#type' => 'textfield',
    '#title' => t('Refund amount'),
    '#description' => $description,
    '#default_value' => $balance_amount,
    '#field_suffix' => check_plain($transaction->currency_code),
    '#size' => 16,
  );

  $form = confirm_form($form,
    t('What amount do you want to refund?'),
    'admin/commerce/orders/' . $order->order_id . '/payment',
    '',
    t('Refund'),
    t('Cancel'),
    'confirm'
  );

  return $form;
}

/**
 * Validate handler: ensure a valid amount is given.
 */
function commerce_moneris_api_refund_form_validate($form, &$form_state) {
  $transaction = $form_state['transaction'];
  $amount = $form_state['values']['amount'];

  // Ensure a positive numeric amount has been entered for capture.
  if (!is_numeric($amount) || $amount <= 0) {
    form_set_error('amount', t('You must specify a positive numeric amount to refund.'));
  }

  // Ensure the amount is less than or equal to the authorization amount.
  if ($amount > $form_state['max_refund_amount']) {
    form_set_error('amount', t('You cannot refund more than you captured through Moneris.'));
  }
}

/**
 * Submit handler: process a prior authorization capture via AIM.
 */
function commerce_moneris_api_refund_form_submit($form, &$form_state) {
  $transaction = $form_state['transaction'];
  $amount = $form_state['values']['amount'];

  $response = commerce_moneris_api_refund_transaction($transaction, $amount);

  $form_state['redirect'] = 'admin/commerce/orders/' . $form_state['order']->order_id . '/payment';
}
