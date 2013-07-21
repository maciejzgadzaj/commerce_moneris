1. Canada Gateway

1.1 Sandbox testing
Configure Moneris settings: 
/admin/commerce/config/payment-methods/manage/commerce_payment_commerce_moneris
  - Select Canada Geteway
  - Select Moneris Sandbox 
  - Store ID: store1
  - API Token: yesguy


1.2 Testing credit card 
On checkout form, select Moneris and use the following:
  - Credit Card Number: 4242424242424242 or 5454545454545454.
  - Select Expiration Month/Year with a pair values from the future.
  - CVD - three digits. For example 123.


1.3 Checking the transaction on moneris sandbox 
  - Go to moneris sandbox page: https://esqa.moneris.com/mpg/index.php
  - Login with: demouser/store1/password
  - Once logged in, go to Reports->Transactions and perform a Search
  - You will see a listing with all transactions 
  (yours and other users that are testing moneris)



2. US Getway

2.1 Sandbox testing
Configure Moneris settings: 
/admin/commerce/config/payment-methods/manage/commerce_payment_commerce_moneris
  - Select US Geteway
  - Select Moneris Sandbox 
  - Store ID: monusqa002
  - API Token: qatoken


2.2 Testing credit card 
On checkout form, select Moneris and use the following:
  - Credit Card Number: 4242424242424242 or 5454545454545454.
  - Select Expiration Month/Year with a pair values from the future.
  - CVD - three digits. For example 123.

2.3 Checking the transaction on moneris sandbox 
  - Go to moneris sandbox page: https://esplusqa.moneris.com/usmpg/index.php
  - Login with: demouser/monsqa002/abc1234
  - Once logged in, go to Reports->Transactions and perform a Search
  - You will see a listing with all transactions 
  (yours and other users that are testing moneris)



3. Alter data that is sent to Moneris
Use a function like:

  function mymodule_commerce_moneris_txnarray_alter(&$txn_array, $order) {
    ...
    $txn_array['order_id'] = $my_order_id;
    ...
  }

to alter the values that are sent to Moneris.
You can use this to override the order_id or customer_id.


4. When you are using SSL, you need to download the cacert.pem 
and upload it to your server (if it is not already there).
On the configuration page, you have to set the path to that file.
This is used for cURL to make SSL requests to Moneris. 


5. Moneris HPP - Hosted Paypage
This is a different Pyament and needs to be configured accordingly.
You will need a hpp_id and hpp_key from Moneris.

You also have to configure your store to do a POST to:
http://yoursite/commerce-moneris-hpp/callback
