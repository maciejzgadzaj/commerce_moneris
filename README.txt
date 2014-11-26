
DESCRIPTION
===========

Moneris (http://www.moneris.com) integration modules
for the Drupal Commerce payment and checkout system.

Commerce Moneris comes with 2 submodules, providing integration with
2 independent Moneris payment solutions:

1) API Integration Method

   On-site payment method provided by Commerce Moneris API module.

   Detailed payment solution description is available
   on the Moneris Solutions Developer Portal:

   - US: https://developer.moneris.com/Downloads/US/APIs.aspx
   - Canada: https://developer.moneris.com/Downloads/Canada/API.aspx

2) Hosted Pay Page Method

   Off-site payment method provided by Commerce Moneris HPP module.

   Detailed payment solution description is available
   on the Moneris Solutions Developer Portal:

   - US: https://developer.moneris.com/Downloads/US/Hosted%20Pay%20Page.aspx
   - Canada: https://developer.moneris.com/Downloads/Canada/Hosted%20Pay%20Page.aspx



TRANSACTION CURRENCIES

Both gateways support only one currency native to their relevant country - USD
for US gateway and CAD in case of CA gateway. If a transaction is being made
and the payment requested in any other currency, before sending it to the
gateway it will be converted to the available gateway currency using Drupal
Commerce currency conversion functionality (commerce_currency_convert()).

See https://drupalcommerce.org/user-guide/currency-conversion for more info.




INSTALLATION
============

Please refer to the README.txt file available in relevant module directory
on how to install and configure each module and provided payment method.
