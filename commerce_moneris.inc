<?php 

/**
 * @file
 * Class that will handle the Moneris Payment.
 *
 */

class CommerceMoneris {
  var $globals;

  var $gateway;
  var $store_id;
  var $api_token;

  var $commerce_moneris_response;

  /**
   * Constructor.
   * @param string $gateway
   * 	 The gateway used for payment ca/us.
   * @param string $store_id
   * 	 The store_id.
   * @param string $api_token
   * 	 The api_token.
   */
  function CommerceMoneris($gateway, $store_id, $api_token) {
    $this->gateway = $gateway;
    $this->store_id = $store_id;
    $this->api_token = $api_token;

    switch ($gateway) {
      case 'ca':
        $this->globals = array(
          'MONERIS_PROTOCOL' => 'https',
          'MONERIS_HOST' => 'www3.moneris.com',
          'MONERIS_TEST_HOST' => 'esqa.moneris.com',
          'MONERIS_PORT' => '443',
          'MONERIS_FILE' => '/gateway2/servlet/MpgRequest',
          'API_VERSION'  => 'MpgApi Version 2.03(php)',
          'CLIENT_TIMEOUT' => '60',
        );
        break;

      case 'us':
        $this->globals = array(
          'MONERIS_PROTOCOL' => 'https',
          'MONERIS_HOST' => 'esplus.moneris.com',
          'MONERIS_TEST_HOST' => 'esplusqa.moneris.com',
          'MONERIS_PORT' => '443',
          'MONERIS_FILE' => '/gateway_us/servlet/MpgRequest',
          'API_VERSION'  => 'MPG version 1.0',
          'CLIENT_TIMEOUT' => '60',
        );
        break;
    }
  }

  /**
   * Post data to Moneris Gateway using cURL library.
   * 
   * @param string $type
   * 	 The type of the transaction.
   * @param Array $txn_array
   * 	 Data that will be send to Moneris.
   * @param Boolean $production
   * 	 Flag indicating if the request goes to Moneris production or sandbox.
   * @param Boolean $ssl
   * 	 Flag indicating if we are using SSL connection.
   * @param unknown_type $cacert_path
   * 	 The path to cacert.pem in case $ssl is TRUE.
   */
  public function post($type, $txn_array, $production = TRUE, $ssl = FALSE, $cacert_path = '') {
    $xml = $this->getXML($type, $txn_array);

    // Post the xml to Moneris.
    $url = $this->globals['MONERIS_PROTOCOL'] . "://";
    $url .= ($production ? $this->globals['MONERIS_HOST'] : $this->globals['MONERIS_TEST_HOST']) . ":";
    $url .= $this->globals['MONERIS_PORT'] . $this->globals['MONERIS_FILE'];

    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, $url);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
    curl_setopt($ch, CURLOPT_HEADER, 0);
    curl_setopt($ch, CURLOPT_POST, 1);
    curl_setopt($ch, CURLOPT_POSTFIELDS, $xml);
    curl_setopt($ch, CURLOPT_TIMEOUT, $this->globals['CLIENT_TIMEOUT']);
    curl_setopt($ch, CURLOPT_USERAGENT, $this->globals['API_VERSION']);

    // When ssl is usign, include the cacert.pem .
    if ($ssl) {
      curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, TRUE);
      curl_setopt($ch, CURLOPT_CAINFO, $cacert_path);
    }
    else {
      curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, FALSE);
    }

    $response = curl_exec($ch);

    // Check for cURL errors.
    $error_no = curl_errno($ch);
    if ($error_no) {
      $error_message = curl_error($ch);

      $result['error_no'] = $error_no;
      $result['error_message'] = $error_message;
    }

    curl_close($ch);

    if (!$response && !$error_no) {
       $response = "<?xml version=\"1.0\"?>
       			<response><receipt>  
              <ReceiptId>Global Error Receipt</ReceiptId> 
              <ReferenceNum>null</ReferenceNum><ResponseCode>null</ResponseCode> 
              <ISO>null</ISO> <AuthCode>null</AuthCode><TransTime>null</TransTime> 
              <TransDate>null</TransDate><TransType>null</TransType><Complete>false</Complete> 
              <Message>null</Message><TransAmount>null</TransAmount>
              <card_type>null</card_type> 
              <TransID>null</TransID><TimedOut>null</TimedOut> 
            </receipt></response>";

       $result['error_no'] = 1;
       $result['error_message'] = 'Global Error Receipt';
    }

     // Create the response object.
     $this->commerce_moneris_response = new CommerceMonerisResponse($response);

     return $result;
  }

  /**
   * Generates the xml taht will be sent to Monris.
   * 
   * @param String $type
   *   The type of the transaction.
   * @param Array $txn_array
   * 	 Data that will be send to Moneris.
   */
  private function getXML($type, $txn_array) {
    $request = new CommerceMonerisRequest($this->gateway, $txn_array);
    $request_xml = $request->generateXml($type);

    $xml .= "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?> 
            <request> 
            <store_id>$this->store_id</store_id>  
            <api_token>$this->api_token</api_token> 
              $request_xml 
            </request>";

    return $xml;
  }

  /**
   * Get the moneris response object.
   */
  public function getCommerceMonerisResponse() {
    return $this->commerce_moneris_response;
  }
}

/**
 * Class that handels the Moneris Request.
 * @author ionut
 */
class CommerceMonerisRequest {
  var $txn_types;
  var $txn_array;

  /**
   * Constructor of Moneris Request.
   * @param string $gateway
   * 	Gateway that will be used: ca/us
   * @param Array $txn_array
   * 	Data that will be send to Moneris
   */
  function CommerceMonerisRequest($gateway, $txn_array) {
    switch ($gateway) {
      case 'ca':
        $this->txn_types = array(
					'purchase' => array('order_id', 'cust_id', 'amount', 'pan', 'expdate', 'crypt_type', 'cvd_indicator', 'cvd_value'),
        );
        break;

      case 'us':
        $this->txn_types = array(
  				'us_purchase' => array('order_id', 'cust_id', 'amount', 'pan', 'expdate', 'crypt_type', 'commcard_invoice', 'commcard_tax_amount', 'dynamic_descriptor', 'cvd_indicator', 'cvd_value'),
        );
        break;
    }

    $this->txn_array = $txn_array;
  }

  /**
   * Generate the xml for Moneris.
   * 
   * @param String $type
   * 	 Type of transaction.
   */
  function generateXml($type) {
    $xml = "<$type>";
    foreach ($this->txn_types[$type] as $t) {
      if ($this->txn_array[$t]) {
        $xml .= "<$t>" . $this->txn_array[$t] . "</$t>";
      }
    }

    $xml .= "</$type>";

    return $xml;
  }
}

/**
 * Handles the Moneris Response.
 * @author ionut
 */
class CommerceMonerisResponse {
  var $response_data;

  var $p;

  var $current_tag;
  var $purchase_hash = array();
  var $refund_hash;
  var $correction_hash = array();
  var $is_batch_totals;
  var $term_id;
  var $receipt_hash = array();
  var $ecr_hash = array();
  var $card_type;
  var $current_txn_type;
  var $ecrs = array();
  var $cards = array();
  var $card_hash = array();

  var $acs_url;

  /**
   * Constructor of CommerceMonerisResponse
   * @param string $xml_string
   * 	The xml string returned by Moneris.
   */
  function CommerceMonerisResponse($xml_string) {
    $this->p = xml_parser_create();
    xml_parser_set_option($this->p, XML_OPTION_CASE_FOLDING, 0);
    xml_parser_set_option($this->p, XML_OPTION_TARGET_ENCODING, "UTF-8");
    xml_set_object($this->p, $this);
    xml_set_element_handler($this->p, "startHandler", "end_handler");
    xml_set_character_data_handler($this->p, "characterHandler");
    xml_parse($this->p, $xml_string);
    xml_parser_free($this->p);
  }

  /**
   * Returns the response data.
   */
  public function getResponseData() {
    return $this->response_data;
  }

  /**
   * Returns the respone code.
   */
  function getResponseCode() {  
    return $this->response_data['ResponseCode'];  
  }
  
  /**
   * Returns the response message.
   */
  function getMessage() {
    return $this->response_data['Message'];
  }
  
  /**
   * Custom function used for parsing.
   * 
   * @param Object $parser
   * 	 The parser.
   * @param Object $data
   * 	 The data.
   */
  function characterHandler($parser, $data) {
   if ($this->is_batch_totals) {
     switch ($this->current_tag) {
       case "term_id"    :
         $this->term_id = $data;
         array_push($this->ecrs, $this->term_id);
         $this->card_hash[$data] = array();
         break;
  
      case "closed"     :
        $ecr_hash=$this->ecr_hash;
        $ecr_hash[$this->term_id] = $data;
        $this->ecr_hash = $ecr_hash;
        break;

      case "card_type"   :
        $this->card_type = $data;
        $this->cards[$data] = $data;
        array_push($this->card_hash[$this->term_id], $data);
        break;

      case "Amount"     :
        if ($this->current_txn_type == "Purchase") {
           $this->purchase_hash[$this->term_id][$this->card_type]['Amount'] = $data;
        }
        elseif ( $this->current_txn_type == "Refund") {
          $this->refund_hash[$this->term_id][$this->card_type]['Amount'] = $data;
        }
        elseif ( $this->current_txn_type == "Correction") {
          $this->correction_hash[$this->term_id][$this->card_type]['Amount'] = $data;
        }
        break;

      case "Count"     :
        if ($this->current_txn_type == "Purchase") {
           $this->purchase_hash[$this->term_id][$this->card_type]['Count'] = $data;
        }
        elseif ( $this->current_txn_type == "Refund") {
          $this->refund_hash[$this->term_id][$this->card_type]['Count'] = $data;
        }
        elseif ( $this->current_txn_type == "Correction") {
          $this->correction_hash[$this->term_id][$this->card_type]['Count'] = $data;
        }
        break;
     }
   }
   else {
      $this->response_data[$this->current_tag] .= $data;
   }
  }

  /**
   * Custom function used for parser.
   * 
   * @param Object $parser
   * 	 The parser.
   * @param String $name
   * 	 The name.
   * @param Array $attrs
   * 	 The attributes.
   */
  function startHandler($parser, $name, $attrs) {  
    $this->current_tag = $name;
      
    if ($this->current_tag == "BankTotals") {
      $this->is_batch_totals = 1;
    }
    elseif ($this->current_tag == "Purchase") {
      $this->purchase_hash[$this->term_id][$this->card_type] = array();
      $this->current_txn_type = "Purchase";
    }
    elseif ($this->current_tag == "Refund") {
      $this->refund_hash[$this->term_id][$this->card_type] = array();
      $this->current_txn_type = "Refund";
    }
    elseif ($this->current_tag == "Correction") {
      $this->correction_hash[$this->term_id][$this->card_type] = array();
      $this->current_txn_type = "Correction";
    }
  }

  /**
   * Custom function used for parser.
   * 
   * @param Object $parser
   * 	 The parser.
   * @param String $name
   * 	 The name.
   */
  function end_handler($parser, $name) {
    $this->current_tag = $name;
    if ($name == "BankTotals") {
      $this->is_batch_totals = 0;
    }

    $this->current_tag="/dev/null";
  }
}
