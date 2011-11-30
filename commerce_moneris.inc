<?php 

/**
 * @file
 * Enter description here ...
 * 
 */

class commerceMoneris {
  var $globals;
  
  var $gateway;
  var $store_id;
  var $api_token;
  
  var $commerceMonerisResponse;
  
  function commerceMoneris($gateway, $store_id, $api_token) {
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
          'CLIENT_TIMEOUT' => '60'
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
          'CLIENT_TIMEOUT' => '60'
        );         
        break;
    }        
  }
  
  public function post($type, $txnArray, $production = TRUE, $ssl = FALSE, $cacert_path = '') {
    $xml = $this->getXML($type, $txnArray);    
    
    // Post the xml to Moneris.        
    $url =  $this->globals['MONERIS_PROTOCOL'] . "://";
    $url .= ($production ? $this->globals['MONERIS_HOST'] : $this->globals['MONERIS_TEST_HOST']) . ":";
    $url .= $this->globals['MONERIS_PORT'] . $this->globals['MONERIS_FILE'];
         
    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, $url);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER,   1);
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
       $response = "<?xml version=\"1.0\"?><response><receipt>" . 
            "<ReceiptId>Global Error Receipt</ReceiptId>" . 
            "<ReferenceNum>null</ReferenceNum><ResponseCode>null</ResponseCode>" . 
            "<ISO>null</ISO> <AuthCode>null</AuthCode><TransTime>null</TransTime>" . 
            "<TransDate>null</TransDate><TransType>null</TransType><Complete>false</Complete>" . 
            "<Message>null</Message><TransAmount>null</TransAmount>" . 
            "<CardType>null</CardType>" . 
            "<TransID>null</TransID><TimedOut>null</TimedOut>" . 
            "</receipt></response>";      

       $result['error_no'] = 1;
       $result['error_message'] = 'Global Error Receipt';       
     }
     
     // Create the response object.
     $this->commerceMonerisResponse = new commerceMonerisResponse($response);
      
     return $result;        
  }
  
  private function getXML($type, $txnArray) {
    $request = new commerceMonerisRequest($this->gateway, $txnArray);
    $requestXML = $request->generateXML($type);  
    
    $xml .= "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>" . 
            "<request>" . 
            "<store_id>$this->store_id</store_id>" . 
            "<api_token>$this->api_token</api_token>" 
            . $requestXML . 
            "</request>";
        
    return $xml;
  }   

  public function getCommerceMonerisResponse() {
    return $this->commerceMonerisResponse;
  }
}


class commerceMonerisRequest {
  var $txnTypes; 
  var $txnArray;
  
  function commerceMonerisRequest($gateway, $txnArray) {
    switch ($gateway) {
      case 'ca':
        $this->txnTypes = array(
  'purchase' => array('order_id', 'cust_id', 'amount', 'pan', 'expdate', 'crypt_type', 'cvd_indicator', 'cvd_value')
        );
      break;
      
      case 'us':
        $this->txnTypes = array(
  'us_purchase' => array('order_id', 'cust_id', 'amount', 'pan', 'expdate', 'crypt_type', 'commcard_invoice', 'commcard_tax_amount', 'dynamic_descriptor', 'cvd_indicator', 'cvd_value')
        );
      break;
    }
    
    $this->txnArray = $txnArray;
  }
  
  /**
   * 
   * Generate the xml from data values.  
   */
  function generateXML($type) {
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

class commerceMonerisResponse {
  var $responseData;
  
  var $p;
  
  var $currentTag;
  var $purchaseHash = array();
  var $refundHash;
  var $correctionHash = array();
  var $isBatchTotals;
  var $term_id;
  var $receiptHash = array();
  var $ecrHash = array();
  var $CardType;
  var $currentTxnType;
  var $ecrs = array();
  var $cards = array();
  var $cardHash= array();
  
  var $ACSUrl;

  function commerceMonerisResponse($xmlString) {
    $this->p = xml_parser_create();
    xml_parser_set_option($this->p, XML_OPTION_CASE_FOLDING, 0);
    xml_parser_set_option($this->p, XML_OPTION_TARGET_ENCODING, "UTF-8");
    xml_set_object($this->p, $this);
    xml_set_element_handler($this->p, "startHandler", "endHandler");
    xml_set_character_data_handler($this->p, "characterHandler");
    xml_parse($this->p, $xmlString);
    xml_parser_free($this->p);
  }

  public function getResponseData() {
    return $this->responseData;
  }

  function getResponseCode() {  
    return $this->responseData['ResponseCode'];  
  }  
  
  function getMessage() {  
    return $this->responseData['Message'];  
  }  
  
  function characterHandler($parser, $data) {
   if ($this->isBatchTotals)  {
     switch ($this->currentTag) {
       case "term_id"    : 
        $this->term_id = $data;
        array_push($this->ecrs, $this->term_id);
        $this->cardHash[$data] = array();
      break;
  
      case "closed"     : 
        $ecrHash=$this->ecrHash;
        $ecrHash[$this->term_id] = $data;
        $this->ecrHash = $ecrHash;
      break;
      
      case "CardType"   : 
        $this->CardType = $data;
        $this->cards[$data] = $data;
        array_push($this->cardHash[$this->term_id], $data) ;
      break;
      case "Amount"     :   
        if ($this->currentTxnType == "Purchase") {
           $this->purchaseHash[$this->term_id][$this->CardType]['Amount'] = $data;
        }
        elseif ( $this->currentTxnType == "Refund") {
          $this->refundHash[$this->term_id][$this->CardType]['Amount'] = $data;
        }
        elseif ( $this->currentTxnType == "Correction") {
          $this->correctionHash[$this->term_id][$this->CardType]['Amount'] = $data;
        }
      break;
  
      case "Count"     : 
        if ($this->currentTxnType == "Purchase") {
           $this->purchaseHash[$this->term_id][$this->CardType]['Count'] = $data;
        }
        elseif ( $this->currentTxnType == "Refund") {
          $this->refundHash[$this->term_id][$this->CardType]['Count'] = $data;
        }
        elseif ( $this->currentTxnType == "Correction") {
          $this->correctionHash[$this->term_id][$this->CardType]['Count'] = $data;
        }
      break;
     }
   }
   else {
      $this->responseData[$this->currentTag] .= $data;
   }
  }

  function startHandler($parser, $name, $attrs) {
  
    $this->currentTag = $name;
      
    if ($this->currentTag == "BankTotals") {
      $this->isBatchTotals = 1;
    }
    elseif ($this->currentTag == "Purchase") {
      $this->purchaseHash[$this->term_id][$this->CardType] = array();
      $this->currentTxnType = "Purchase";
    }
    elseif ($this->currentTag == "Refund") {
      $this->refundHash[$this->term_id][$this->CardType] = array();
      $this->currentTxnType = "Refund";
     }
    elseif ($this->currentTag == "Correction") {
      $this->correctionHash[$this->term_id][$this->CardType] = array();
      $this->currentTxnType = "Correction";
    }
  }

  function endHandler($parser, $name) {
    $this->currentTag = $name;
    if ($name == "BankTotals") {
      $this->isBatchTotals = 0;
    }
    
    $this->currentTag="/dev/null";
  }
}
