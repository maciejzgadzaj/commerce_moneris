<?php 

/**
 * @file
 * Class that will handle the Moneris Payment.
 */

class CommerceMoneris {
  protected $globals;

  protected $gateway;
  protected $storeId;
  protected $apiToken;

  protected $commerceMonerisResponse;

  /**
   * Constructor.
   *
   * @param string $gateway
   *   The gateway used for payment ca/us.
   * @param string $store_id
   *   The storeId.
   * @param string $api_token
   *   The apiToken.
   */
  function __construct($gateway, $store_id, $api_token) {
    $this->gateway = $gateway;
    $this->storeId = $store_id;
    $this->apiToken = $api_token;

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
   *   The type of the transaction.
   * @param Array $txn_array
   *   Data that will be send to Moneris.
   * @param Boolean $production
   *   Flag indicating if the request goes to Moneris production or sandbox.
   * @param Boolean $ssl
   *   Flag indicating if we are using SSL connection.
   * @param string $cacert_path
   *   The path to cacert.pem in case $ssl is TRUE.
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

    $result = array();

    // Check for cURL errors.
    $errorno = curl_errno($ch);
    if ($errorno) {
      $result['error_no'] = $errorno;
      $result['error_message'] = curl_error($ch);
    }

    curl_close($ch);

    if (!$response && !$errorno) {
       $response = "<?xml version=\"1.0\"?>
	    <response><receipt>  
              <receiptid>Global Error Receipt</receiptid> 
              <referencenum>null</referencenum>
              <responsecode>null</responsecode> 
              <iso>null</iso>
              <authcode>null</authcode>
              <transtime>null</transtime> 
              <transdate>null</transdate>
              <transtype>null</transtype>
              <complete>false</complete> 
              <message>null</message>
              <transamount>null</transamount>
              <cardType>null</cardType> 
              <transid>null</transid>
              <timedout>null</timedout> 
            </receipt></response>";

       $result['error_no'] = 1;
       $result['error_message'] = 'Global Error Receipt';
    }

     // Create the response object.
     $this->commerceMonerisResponse = new CommerceMonerisResponse($response);

     return $result;
  }

  /**
   * Generates the xml taht will be sent to Monris.
   *
   * @param String $type
   *   The type of the transaction.
   * @param Array $txn_array
   *   Data that will be send to Moneris.
   */
  protected function getXML($type, $txn_array) {
    $request = new CommerceMonerisRequest($this->gateway, $txn_array);
    $request_xml = $request->generateXml($type);

    $xml = "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?> 
            <request> 
            <store_id>$this->storeId</store_id>  
            <api_token>$this->apiToken</api_token> 
              $request_xml 
            </request>";

    return $xml;
  }

  /**
   * Get the moneris response object.
   */
  public function getCommerceMonerisResponse() {
    return $this->commerceMonerisResponse;
  }
}

/**
 * Class that handels the Moneris Request.
 * @author ionut
 */
class CommerceMonerisRequest {
  protected $txnTypes;
  protected $txnArray;

  /**
   * Constructor of Moneris Request.
   *
   * @param string $gateway
   *   Gateway that will be used: ca/us
   * @param Array $txn_array
   *   Data that will be send to Moneris
   */
  function __construct($gateway, $txn_array) {
    switch ($gateway) {
      case 'ca':
        $this->txnTypes = array(
          'purchase' => array(
            'order_id',
            'cust_id',
            'amount',
            'pan',
            'expdate',
            'crypt_type',
            'cvd_indicator',
            'cvd_value',
          ),
        );
        break;

      case 'us':
        $this->txnTypes = array(
          'us_purchase' => array(
            'order_id',
            'cust_id',
            'amount',
            'pan',
            'expdate',
            'crypt_type',
            'commcard_invoice',
            'commcard_tax_amount',
            'dynamic_descriptor',
            'cvd_indicator',
            'cvd_value',
          ),
        );
        break;
    }

    $this->txnArray = $txn_array;
  }

  /**
   * Generate the xml for Moneris.
   *
   * @param String $type
   *   Type of transaction.
   */
  public function generateXml($type) {
    $xml = "<$type>";
    foreach ($this->txnTypes[$type] as $t) {
      if ($this->txnArray[$t]) {
        $xml .= "<$t>" . $this->txnArray[$t] . "</$t>";
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
  protected $responseData;

  protected $p;

  protected $currentTag;
  protected $purchaseHash = array();
  protected $refundHash;
  protected $correctionHash = array();
  protected $isBatchTotals;
  protected $termId;
  protected $receiptHash = array();
  protected $ecrHash = array();
  protected $cardType;
  protected $currentTxnType;
  protected $ecrs = array();
  protected $cards = array();
  protected $cardHash = array();

  protected $acsUrl;

  /**
   * Constructor of CommerceMonerisResponse.
   *
   * @param string $xml_string
   *   The xml string returned by Moneris.
   */
  function __construct($xml_string) {
    $this->p = xml_parser_create();
    xml_parser_set_option($this->p, XML_OPTION_CASE_FOLDING, 0);
    xml_parser_set_option($this->p, XML_OPTION_TARGET_ENCODING, "UTF-8");
    xml_set_object($this->p, $this);
    xml_set_element_handler($this->p, "startHandler", "endHandler");
    xml_set_character_data_handler($this->p, "characterHandler");
    xml_parse($this->p, $xml_string);
    xml_parser_free($this->p);
  }

  /**
   * Returns the response data.
   */
  public function getResponseData() {
    return $this->responseData;
  }

  /**
   * Returns the respone code.
   */
  public function getResponseCode() {
    return $this->responseData['ResponseCode'];
  }

  /**
   * Returns the response message.
   */
  public function getMessage() {
    return $this->responseData['Message'];
  }

  /**
   * Custom function used for parsing.
   *
   * @param Object $parser
   *   The parser.
   * @param Object $data
   *   The data.
   */
  protected function characterHandler($parser, $data) {
    if ($this->isBatchTotals) {
      switch ($this->currentTag) {
        case "term_id":
          $this->termId = $data;
          array_push($this->ecrs, $this->termId);
          $this->cardHash[$data] = array();
          break;

        case "closed":
          $ecr_hash = $this->ecrHash;
          $ecr_hash[$this->termId] = $data;
          $this->ecrHash = $ecr_hash;
          break;

        case "card_type":
          $this->cardType = $data;
          $this->cards[$data] = $data;
          array_push($this->cardHash[$this->termId], $data);
          break;

        case "amount":
          if ($this->currentTxnType == "Purchase") {
            $this->purchaseHash[$this->termId][$this->cardType]['Amount'] = $data;
          }
          elseif ($this->currentTxnType == "Refund") {
            $this->refundHash[$this->termId][$this->cardType]['Amount'] = $data;
          }
          elseif ($this->currentTxnType == "Correction") {
            $this->correctionHash[$this->termId][$this->cardType]['Amount'] = $data;
          }
          break;

        case "count":
          if ($this->currentTxnType == "Purchase") {
            $this->purchaseHash[$this->termId][$this->cardType]['Count'] = $data;
          }
          elseif ($this->currentTxnType == "Refund") {
            $this->refundHash[$this->termId][$this->cardType]['Count'] = $data;
          }
          elseif ($this->currentTxnType == "Correction") {
            $this->correctionHash[$this->termId][$this->cardType]['Count'] = $data;
          }
          break;
      }
    }
    else {
      $this->responseData[$this->currentTag] = $data;
    }
  }

  /**
   * Custom function used for parser.
   *
   * @param Object $parser
   *   The parser.
   * @param String $name
   *   The name.
   * @param Array $attrs
   *   The attributes.
   */
  protected function startHandler($parser, $name, $attrs) {
    $this->currentTag = $name;

    if ($this->currentTag == "BankTotals") {
      $this->isBatchTotals = 1;
    }
    elseif ($this->currentTag == "Purchase") {
      $this->purchaseHash[$this->termId][$this->cardType] = array();
      $this->currentTxnType = "Purchase";
    }
    elseif ($this->currentTag == "Refund") {
      $this->refundHash[$this->termId][$this->cardType] = array();
      $this->currentTxnType = "Refund";
    }
    elseif ($this->currentTag == "Correction") {
      $this->correctionHash[$this->termId][$this->cardType] = array();
      $this->currentTxnType = "Correction";
    }
  }

  /**
   * Custom function used for parser.
   *
   * @param Object $parser
   *   The parser.
   * @param String $name
   *   The name.
   */
  protected function endHandler($parser, $name) {
    $this->currentTag = $name;
    if ($name == "BankTotals") {
      $this->isBatchTotals = 0;
    }

    $this->currentTag = "/dev/null";
  }
}
