<?php
// $Id$

/**
 * @file
 * OpenID SREG utility functions.
 * 
 * @author
 * Stefan Auditor <stefan.auditor@erdfisch.de>
 */

// OpenID Simple Registration (SREG) namespace URL
define('OPENID_SREG_1_1', 'http://openid.net/extensions/sreg/1.1');

/**
 * Generate a SREG request
 */
function openid_sreg_generate_request($query = array()) {
  $request = array();

  $request['openid.ns.sreg'] = OPENID_SREG_1_1;
  if ($required = variable_get('openid_sreg_request_required', 'nickname,email')) {
    $request['openid.sreg.required'] = $required;
  }
  if ($optional = variable_get('openid_sreg_request_optional', '')) {
    $request['openid.sreg.optional'] = $optional;
  }
  if ($policy_url = variable_get('openid_sreg_request_policy_url', FALSE)) {
    $request['openid.sreg.policy_url'] = url($policy_url, array('absolute' => TRUE));
  }
  return $request;
}

/**
 * Generate a SREG response
 */
function openid_sreg_generate_response($request = array(), $response = array(), $user = NULL) {
  static $query = array();
  if (!$user) {
    global $user;
  }

  if (!$query) {
    // Namespace declaration
    $query['openid.ns.sreg'] = OPENID_SREG_1_1;
    
    // Collect attributes
    $attributes = openid_sreg_user_get_attributes($user);
    // Change date format
    $attributes['dob'] = $attributes['dob']['year'] .'-'. sprintf("%02d", $attributes['dob']['month']) .'-'. sprintf("%02d", $attributes['dob']['day']);
    
    foreach ($attributes as $key => $value) {
      $query['openid.sreg.'. $key] = $value;
    }
  }

  return $query;
}

/**
 * Generate keys from a query array
 */
function openid_sreg_generate_keys($query = array()) {
  $keys = array();
  foreach ($query as $key => $value) {
    $keys[] = str_replace('openid.', '', $key);
  }
  return $keys;
}

/**
 * Load a user's attributes
 */
function openid_sreg_load_attributes($user) {
  $attributes = array();
  $result = db_query("SELECT identifier, value FROM {openid_sreg} WHERE uid=%d", $user->uid);
  while ($row = db_fetch_object($result)) {
    $attributes[$row->identifier] = $row->value;
  }
  $attributes['dob'] = unserialize($attributes['dob']);
  return $attributes;
}

/**
 * Save a user's attributes
 */
function openid_sreg_save_attributes($uid, $attributes) {
  $allowed_attributes = openid_sreg_get_attributes();
  
  foreach ($attributes as $identifier => $value) {
    // Normalize data
    $identifier = str_replace('openid.sreg.', '', $identifier);
    // Save only valid and filled out attributes
    if (isset($allowed_attributes['openid.sreg.'. $identifier]) AND !empty($value)) {
      if (is_array($value)) {
        // Serialize dob attribute
        $value = serialize($value);
      }
      db_query("INSERT INTO {openid_sreg} (uid, identifier, value) VALUES (%d, '%s', '%s')", $uid, $identifier, $value);
    }
  }
}

/**
 * Collect a user's attributes
 */
function openid_sreg_user_get_attributes($user, $attribute = NULL) {
  static $attributes = array();
  
  if (!$attributes) {
    $attributes = openid_sreg_load_attributes($user);
    
    if (!$attributes['nickname']) {
      $attributes['nickname'] = $user->name;
    }
    if (!$attributes['email']) {
      $attributes['email']    = $user->mail;
    }
    
    //TODO: Integration with profile and cck
    $attributes['fullname'];
    $attributes['dob'];
    $attributes['gender'];
    $attributes['postcode'];
    
    if (!$attributes['country']) {
      $attributes['country'] = openid_sreg_user_get_country_code($user);
    }
    if (!$attributes['language']) {
      // Language and timezone come from Drupal core
      $attributes['language'] = openid_sreg_user_get_language($user);
    }
    
    // It seems not possible to reliably determine a user's time zone in the desired format
    $attributes['timezone'];
  }

  return $attribute ? $attributes[$attribute] : $attributes;
}

/**
 * Determine a user's country
 */
function openid_sreg_user_get_country_code($user) {
  // Check for user specific language setting
  if ($user->language) {
    $country_code = $user->language;
  }
  else {
    // Or use default language used on the site
    $lang = language_default();
    $country_code = $lang->language;
  }
  return strtoupper($country_code);
}

/**
 * Determine a user's preffered language
 */
function openid_sreg_user_get_language($user) {
  if ($user->language) {
    // Check for user specific language setting
    $language = $user->language;
  }
  else {
    // Or use default language used on the site
    $lang = language_default();
    $language = $lang->language;
  }
  return $language;
}

/**
 * Extract sreg.required or sreg.optional values from query string
 */
function openid_sreg_query_get_fields($query, $mode = 'required') {
  $attributes = array();
  if (isset($query['openid.sreg.'. $mode])) {
    $attributes = explode(',', $query['openid.sreg.'. $mode]);
  }
  return $attributes;
}

/**
 * Prepares the language code list from drupal core for a select form
 */
function openid_sreg_locale_prepare_predefined_list() {
  include_once './includes/locale.inc';
  
  $predefined = _locale_get_predefined_list();
  foreach ($predefined as $key => $value) {
    // Include native name in output, if possible
    if (count($value) > 1) {
      $tname = t($value[0]);
      $predefined[$key] = ($tname == $value[1]) ? $tname : "$tname ($value[1])";
    }
    else {
      $predefined[$key] = t($value[0]);
    }
  }
  asort($predefined);
  return $predefined;
}

/**
 * Get the defined attributes
 * See http://openid.net/specs/openid-simple-registration-extension-1_1-01.html#rfc.section.4
 * See http://www.axschema.org/types/
 */
function openid_sreg_get_attributes() {
  $attributes = array(
    'openid.sreg.nickname' => array(
      'type_URI'    => 'http://axschema.org/namePerson/friendly',
      'label'       => 'Alias',
      'description' => 'Subject\'s alias or "screen" name',
      'example'     => '"Johnny5"',
      'type'        => 'http://www.w3.org/2001/XMLSchema#normalizedString',
    ),
    'openid.sreg.email' => array(
      'type_URI'    => 'http://axschema.org/contact/email',
      'label'       => 'Email',
      'description' => 'Internet SMTP email address',
      'example'     => '"jsmith@isp.example.com"',
      'type'        => 'http://www.w3.org/2001/XMLSchema#normalizedString',
    ),
    'openid.sreg.fullname' => array(
      'type_URI'    => 'http://axschema.org/namePerson',
      'label'       => 'Full name',
      'description' => 'Full name of subject',
      'example'     => '"John Doe"',
      'type'        => 'http://www.w3.org/2001/XMLSchema#normalizedString',
    ),
    'openid.sreg.dob' => array(
      'type_URI'    => 'http://axschema.org/birthDate',
      'label'       => 'Birth date',
      'description' => 'Date of birth',
      'example'     => '"1979-01-01"',
      'type'        => 'http://www.w3.org/2001/XMLSchema#date',
    ),
    'openid.sreg.gender'   => array(
      'type_URI'    => 'http://axschema.org/person/gender',
      'label'       => 'Gender',
      'description' => 'Gender, either "M" or "F"',
      'example'     => '"M", "F"',
      'type'        => 'http://www.w3.org/2001/XMLSchema#normalizedString',
    ),
    'openid.sreg.postcode' => array(
      'type_URI'    => 'http://axschema.org/contact/postalCode/home',
      'label'       => 'Postal code',
      'description' => 'Home postal code',
      'example'     => '"V5A 4B2"',
      'type'        => 'http://www.w3.org/2001/XMLSchema#normalizedString',
    ),
    'openid.sreg.country'  => array(
      'type_URI'    => 'http://axschema.org/contact/country/home      ',
      'label'       => 'Country',
      'description' => 'Home country code',
      'example'     => '"CA"',
      'type'        => 'http://www.w3.org/2001/XMLSchema#normalizedString',
    ),
    'openid.sreg.language' => array(
      'type_URI'    => 'http://axschema.org/pref/language',
      'label'       => 'Language',
      'description' => 'Preferred language',
      'example'     => '"en-US"',
      'type'        => 'http://www.w3.org/2001/XMLSchema#normalizedString',
    ),
    'openid.sreg.timezone' => array(
      'type_URI'    => 'http://axschema.org/pref/timezone',
      'label'       => 'Time zone',
      'description' => 'Home time zone information',
      'example'     => '"America/Pacific"',
      'type'        => 'http://www.w3.org/2001/XMLSchema#normalizedString',
    ),
  );
  return $attributes;
}

/**
 * List of timezones
 * See http://en.wikipedia.org/wiki/List_of_tz_zones_by_name
 */
function openid_sreg_get_timezones() {
  $timezones = array(
    'Africa/Abidjan',
    'Africa/Accra',
    'Africa/Addis_Ababa',
    'Africa/Algiers',
    'Africa/Asmara',
    'Africa/Bamako',
    'Africa/Bangui',
    'Africa/Banjul',
    'Africa/Bissau',
    'Africa/Blantyre',
    'Africa/Brazzaville',
    'Africa/Bujumbura',
    'Africa/Cairo',
    'Africa/Casablanca',
    'Africa/Ceuta',
    'Africa/Conakry',
    'Africa/Dakar',
    'Africa/Dar_es_Salaam',
    'Africa/Djibouti',
    'Africa/Douala',
    'Africa/El_Aaiun',
    'Africa/Freetown',
    'Africa/Gaborone',
    'Africa/Harare',
    'Africa/Johannesburg',
    'Africa/Kampala',
    'Africa/Khartoum',
    'Africa/Kigali',
    'Africa/Kinshasa',
    'Africa/Lagos',
    'Africa/Libreville',
    'Africa/Lome',
    'Africa/Luanda',
    'Africa/Lubumbashi',
    'Africa/Lusaka',
    'Africa/Malabo',
    'Africa/Maputo',
    'Africa/Maseru',
    'Africa/Mbabane',
    'Africa/Mogadishu',
    'Africa/Monrovia',
    'Africa/Nairobi',
    'Africa/Ndjamena',
    'Africa/Niamey',
    'Africa/Nouakchott',
    'Africa/Ouagadougou',
    'Africa/Porto-Novo',
    'Africa/Sao_Tome',
    'Africa/Tripoli',
    'Africa/Tunis',
    'Africa/Windhoek',
    'America/Adak',
    'America/Anchorage',
    'America/Anguilla',
    'America/Antigua',
    'America/Araguaina',
    'America/Argentina/Buenos_Aires',
    'America/Argentina/Catamarca',
    'America/Argentina/Cordoba',
    'America/Argentina/Jujuy',
    'America/Argentina/La_Rioja',
    'America/Argentina/Mendoza',
    'America/Argentina/Rio_Gallegos',
    'America/Argentina/San_Juan',
    'America/Argentina/San_Luis',
    'America/Argentina/Tucuman',
    'America/Argentina/Ushuaia',
    'America/Aruba',
    'America/Asuncion',
    'America/Atikokan',
    'America/Bahia',
    'America/Barbados',
    'America/Belem',
    'America/Belize',
    'America/Blanc-Sablon',
    'America/Boa_Vista',
    'America/Bogota',
    'America/Boise',
    'America/Cambridge_Bay',
    'America/Campo_Grande',
    'America/Cancun',
    'America/Caracas',
    'America/Cayenne',
    'America/Cayman',
    'America/Chicago',
    'America/Chihuahua',
    'America/Costa_Rica',
    'America/Cuiaba',
    'America/Curacao',
    'America/Danmarkshavn',
    'America/Dawson',
    'America/Dawson_Creek',
    'America/Denver',
    'America/Detroit',
    'America/Dominica',
    'America/Edmonton',
    'America/Eirunepe',
    'America/El_Salvador',
    'America/Fortaleza',
    'America/Glace_Bay',
    'America/Godthab',
    'America/Goose_Bay',
    'America/Grand_Turk',
    'America/Grenada',
    'America/Guadeloupe',
    'America/Guatemala',
    'America/Guayaquil',
    'America/Guyana',
    'America/Halifax',
    'America/Havana',
    'America/Hermosillo',
    'America/Indiana/Indianapolis',
    'America/Indiana/Knox',
    'America/Indiana/Marengo',
    'America/Indiana/Petersburg',
    'America/Indiana/Tell_City',
    'America/Indiana/Vevay',
    'America/Indiana/Vincennes',
    'America/Indiana/Winamac',
    'America/Inuvik',
    'America/Iqaluit',
    'America/Jamaica',
    'America/Juneau',
    'America/Kentucky/Louisville',
    'America/Kentucky/Monticello',
    'America/La_Paz',
    'America/Lima',
    'America/Los_Angeles',
    'America/Maceio',
    'America/Managua',
    'America/Manaus',
    'America/Marigot',
    'America/Martinique',
    'America/Mazatlan',
    'America/Menominee',
    'America/Merida',
    'America/Mexico_City',
    'America/Miquelon',
    'America/Moncton',
    'America/Monterrey',
    'America/Montevideo',
    'America/Montreal',
    'America/Montserrat',
    'America/Nassau',
    'America/New_York',
    'America/Nipigon',
    'America/Nome',
    'America/Noronha',
    'America/North_Dakota/Center',
    'America/North_Dakota/New_Salem',
    'America/Panama',
    'America/Pangnirtung',
    'America/Paramaribo',
    'America/Phoenix',
    'America/Port_of_Spain',
    'America/Port-au-Prince',
    'America/Porto_Velho',
    'America/Puerto_Rico',
    'America/Rainy_River',
    'America/Rankin_Inlet',
    'America/Recife',
    'America/Regina',
    'America/Resolute',
    'America/Rio_Branco',
    'America/Santiago',
    'America/Santo_Domingo',
    'America/Sao_Paulo',
    'America/Scoresbysund',
    'America/Shiprock',
    'America/St_Barthelemy',
    'America/St_Johns',
    'America/St_Kitts',
    'America/St_Lucia',
    'America/St_Thomas',
    'America/St_Vincent',
    'America/Swift_Current',
    'America/Tegucigalpa',
    'America/Thule',
    'America/Thunder_Bay',
    'America/Tijuana',
    'America/Toronto',
    'America/Tortola',
    'America/Vancouver',
    'America/Whitehorse',
    'America/Winnipeg',
    'America/Yakutat',
    'America/Yellowknife',
    'Antarctica/Casey',
    'Antarctica/Davis',
    'Antarctica/DumontDUrville',
    'Antarctica/Mawson',
    'Antarctica/McMurdo',
    'Antarctica/Palmer',
    'Antarctica/Rothera',
    'Antarctica/South_Pole',
    'Antarctica/Syowa',
    'Antarctica/Vostok',
    'Arctic/Longyearbyen',
    'Asia/Aden',
    'Asia/Almaty',
    'Asia/Amman',
    'Asia/Anadyr',
    'Asia/Aqtau',
    'Asia/Aqtobe',
    'Asia/Ashgabat',
    'Asia/Baghdad',
    'Asia/Bahrain',
    'Asia/Baku',
    'Asia/Bangkok',
    'Asia/Beirut',
    'Asia/Bishkek',
    'Asia/Brunei',
    'Asia/Choibalsan',
    'Asia/Chongqing',
    'Asia/Colombo',
    'Asia/Damascus',
    'Asia/Dhaka',
    'Asia/Dili',
    'Asia/Dubai',
    'Asia/Dushanbe',
    'Asia/Gaza',
    'Asia/Harbin',
    'Asia/Ho_Chi_Minh',
    'Asia/Hong_Kong',
    'Asia/Hovd',
    'Asia/Irkutsk',
    'Asia/Jakarta',
    'Asia/Jayapura',
    'Asia/Jerusalem',
    'Asia/Kabul',
    'Asia/Kamchatka',
    'Asia/Karachi',
    'Asia/Kashgar',
    'Asia/Katmandu',
    'Asia/Kolkata',
    'Asia/Krasnoyarsk',
    'Asia/Kuala_Lumpur',
    'Asia/Kuching',
    'Asia/Kuwait',
    'Asia/Macau',
    'Asia/Magadan',
    'Asia/Makassar',
    'Asia/Manila',
    'Asia/Muscat',
    'Asia/Nicosia',
    'Asia/Novosibirsk',
    'Asia/Omsk',
    'Asia/Oral',
    'Asia/Phnom_Penh',
    'Asia/Pontianak',
    'Asia/Pyongyang',
    'Asia/Qatar',
    'Asia/Qyzylorda',
    'Asia/Rangoon',
    'Asia/Riyadh',
    'Asia/Sakhalin',
    'Asia/Samarkand',
    'Asia/Seoul',
    'Asia/Shanghai',
    'Asia/Singapore',
    'Asia/Taipei',
    'Asia/Tashkent',
    'Asia/Tbilisi',
    'Asia/Tehran',
    'Asia/Thimphu',
    'Asia/Tokyo',
    'Asia/Ulaanbaatar',
    'Asia/Urumqi',
    'Asia/Vientiane',
    'Asia/Vladivostok',
    'Asia/Yakutsk',
    'Asia/Yekaterinburg',
    'Asia/Yerevan',
    'Atlantic/Azores',
    'Atlantic/Bermuda',
    'Atlantic/Canary',
    'Atlantic/Cape_Verde',
    'Atlantic/Faroe',
    'Atlantic/Madeira',
    'Atlantic/Reykjavik',
    'Atlantic/South_Georgia',
    'Atlantic/St_Helena',
    'Atlantic/Stanley',
    'Australia/Adelaide',
    'Australia/Brisbane',
    'Australia/Broken_Hill',
    'Australia/Currie',
    'Australia/Darwin',
    'Australia/Eucla',
    'Australia/Hobart',
    'Australia/Lindeman',
    'Australia/Lord_Howe',
    'Australia/Melbourne',
    'Australia/Perth',
    'Australia/Sydney',
    'Europe/Amsterdam',
    'Europe/Andorra',
    'Europe/Athens',
    'Europe/Belgrade',
    'Europe/Berlin',
    'Europe/Bratislava',
    'Europe/Brussels',
    'Europe/Bucharest',
    'Europe/Budapest',
    'Europe/Chisinau',
    'Europe/Copenhagen',
    'Europe/Dublin',
    'Europe/Gibraltar',
    'Europe/Guernsey',
    'Europe/Helsinki',
    'Europe/Isle_of_Man',
    'Europe/Istanbul',
    'Europe/Jersey',
    'Europe/Kaliningrad',
    'Europe/Kiev',
    'Europe/Lisbon',
    'Europe/Ljubljana',
    'Europe/London',
    'Europe/Luxembourg',
    'Europe/Madrid',
    'Europe/Malta',
    'Europe/Mariehamn',
    'Europe/Minsk',
    'Europe/Monaco',
    'Europe/Moscow',
    'Europe/Oslo',
    'Europe/Paris',
    'Europe/Podgorica',
    'Europe/Prague',
    'Europe/Riga',
    'Europe/Rome',
    'Europe/Samara',
    'Europe/San_Marino',
    'Europe/Sarajevo',
    'Europe/Simferopol',
    'Europe/Skopje',
    'Europe/Sofia',
    'Europe/Stockholm',
    'Europe/Tallinn',
    'Europe/Tirane',
    'Europe/Uzhgorod',
    'Europe/Vaduz',
    'Europe/Vatican',
    'Europe/Vienna',
    'Europe/Vilnius',
    'Europe/Volgograd',
    'Europe/Warsaw',
    'Europe/Zagreb',
    'Europe/Zaporozhye',
    'Europe/Zurich',
    'Indian/Antananarivo',
    'Indian/Chagos',
    'Indian/Christmas',
    'Indian/Cocos',
    'Indian/Comoro',
    'Indian/Kerguelen',
    'Indian/Mahe',
    'Indian/Maldives',
    'Indian/Mauritius',
    'Indian/Mayotte',
    'Indian/Reunion',
    'Pacific/Apia',
    'Pacific/Auckland',
    'Pacific/Chatham',
    'Pacific/Easter',
    'Pacific/Efate',
    'Pacific/Enderbury',
    'Pacific/Fakaofo',
    'Pacific/Fiji',
    'Pacific/Funafuti',
    'Pacific/Galapagos',
    'Pacific/Gambier',
    'Pacific/Guadalcanal',
    'Pacific/Guam',
    'Pacific/Honolulu',
    'Pacific/Johnston',
    'Pacific/Kiritimati',
    'Pacific/Kosrae',
    'Pacific/Kwajalein',
    'Pacific/Majuro',
    'Pacific/Marquesas',
    'Pacific/Midway',
    'Pacific/Nauru',
    'Pacific/Niue',
    'Pacific/Norfolk',
    'Pacific/Noumea',
    'Pacific/Pago_Pago',
    'Pacific/Palau',
    'Pacific/Pitcairn',
    'Pacific/Ponape',
    'Pacific/Port_Moresby',
    'Pacific/Rarotonga',
    'Pacific/Saipan',
    'Pacific/Tahiti',
    'Pacific/Tarawa',
    'Pacific/Tongatapu',
    'Pacific/Truk',
    'Pacific/Wake',
    'Pacific/Wallis',
  );
  return $timezones;
}

/**
 * List of country codes
 * See http://www.iso.org/iso/country_codes
 */
function openid_sreg_get_country_codes($code = NULL) {
  $country_codes = array(
    'AF' => "Afghanistan",
    'AX' => "Åland Islands",
    'AL' => "Albania",
    'DZ' => "Algeria",
    'AS' => "American Samoa",
    'AD' => "Andorra",
    'AO' => "Angola",
    'AI' => "Anguilla",
    'AQ' => "Antarctica",
    'AG' => "Antigua and Barbuda",
    'AR' => "Argentina",
    'AM' => "Armenia",
    'AW' => "Aruba",
    'AU' => "Australia",
    'AT' => "Austria",
    'AZ' => "Azerbaijan",
    'BS' => "Bahamas",
    'BH' => "Bahrain",
    'BD' => "Bangladesh",
    'BB' => "Barbados",
    'BY' => "Belarus",
    'BE' => "Belgium",
    'BZ' => "Belize",
    'BJ' => "Benin",
    'BM' => "Bermuda",
    'BT' => "Bhutan",
    'BO' => "Bolivia",
    'BA' => "Bosnia and Herzegovina",
    'BW' => "Botswana",
    'BV' => "Bouvet Island",
    'BR' => "Brazil",
    'IO' => "British Indian Ocean Territory",
    'BN' => "Brunei Darussalam",
    'BG' => "Bulgaria",
    'BF' => "Burkina Faso",
    'BI' => "Burundi",
    'KH' => "Cambodia",
    'CM' => "Cameroon",
    'CA' => "Canada",
    'CV' => "Cape Verde",
    'KY' => "Cayman Islands",
    'CF' => "Central African Republic",
    'TD' => "Chad",
    'CL' => "Chile",
    'CN' => "China",
    'CX' => "Christmas Island",
    'CC' => "Cocos (Keeling) Islands",
    'CO' => "Colombia",
    'KM' => "Comoros",
    'CG' => "Congo",
    'CD' => "Congo, The Democratic Republic of the",
    'CK' => "Cook Islands",
    'CR' => "Costa Rica",
    'CI' => "Côte d'Ivoire",
    'HR' => "Croatia",
    'CU' => "Cuba",
    'CY' => "Cyprus",
    'CZ' => "Czech Republic",
    'DK' => "Denmark",
    'DJ' => "Djibouti",
    'DM' => "Dominica",
    'DO' => "Dominican Republic",
    'EC' => "Ecuador",
    'EG' => "Egypt",
    'SV' => "El Salvador",
    'GQ' => "Equatorial Guinea",
    'ER' => "Eritrea",
    'EE' => "Estonia",
    'ET' => "Ethiopia",
    'FK' => "Falkland Islands (Malvinas)",
    'FO' => "Faroe Islands",
    'FJ' => "Fiji",
    'FI' => "Finland",
    'FR' => "France",
    'GF' => "French Guiana",
    'PF' => "French Polynesia",
    'TF' => "French Southern Territories",
    'GA' => "Gabon",
    'GM' => "Gambia",
    'GE' => "Georgia",
    'DE' => "Germany",
    'GH' => "Ghana",
    'GI' => "Gibraltar",
    'GR' => "Greece",
    'GL' => "Greenland",
    'GD' => "Grenada",
    'GP' => "Guadeloupe",
    'GU' => "Guam",
    'GT' => "Guatemala",
    'GG' => "Guernsey",
    'GN' => "Guinea",
    'GW' => "Guinea-Bissau",
    'GY' => "Guyana",
    'HT' => "Haiti",
    'HM' => "Heard Island and McDonald Islands",
    'VA' => "Holy See (Vatican City State)",
    'HN' => "Honduras",
    'HK' => "Hong Kong",
    'HU' => "Hungary",
    'IS' => "Iceland",
    'IN' => "India",
    'ID' => "Indonesia",
    'IR' => "Iran, Islamic Republic of",
    'IQ' => "Iraq",
    'IE' => "Ireland",
    'IM' => "Isle of Man",
    'IL' => "Israel",
    'IT' => "Italy",
    'JM' => "Jamaica",
    'JP' => "Japan",
    'JE' => "Jersey",
    'JO' => "Jordan",
    'KZ' => "Kazakhstan",
    'KE' => "Kenya",
    'KI' => "Kiribati",
    'KP' => "Korea, Democratic People's Republic of",
    'KR' => "Korea, Republic of",
    'KW' => "Kuwait",
    'KG' => "Kyrgyzstan",
    'LA' => "Lao People's Democratic Republic",
    'LV' => "Latvia",
    'LB' => "Lebanon",
    'LS' => "Lesotho",
    'LR' => "Liberia",
    'LY' => "Libyan Arab Jamahiriya",
    'LI' => "Liechtenstein",
    'LT' => "Lithuania",
    'LU' => "Luxembourg",
    'MO' => "Macao",
    'MK' => "Macedonia, The Former Yugoslav Republic of",
    'MG' => "Madagascar",
    'MW' => "Malawi",
    'MY' => "Malaysia",
    'MV' => "Maldives",
    'ML' => "Mali",
    'MT' => "Malta",
    'MH' => "Marshall Islands",
    'MQ' => "Martinique",
    'MR' => "Mauritania",
    'MU' => "Mauritius",
    'YT' => "Mayotte",
    'MX' => "Mexico",
    'FM' => "Micronesia, Federated States of",
    'MD' => "Moldova, Republic of",
    'MC' => "Monaco",
    'MN' => "Mongolia",
    'ME' => "Montenegro",
    'MS' => "Montserrat",
    'MA' => "Morocco",
    'MZ' => "Mozambique",
    'MM' => "Myanmar",
    'NA' => "Namibia",
    'NR' => "Nauru",
    'NP' => "Nepal",
    'NL' => "Netherlands",
    'AN' => "Netherlands Antilles",
    'NC' => "New Caledonia",
    'NZ' => "New Zealand",
    'NI' => "Nicaragua",
    'NE' => "Niger",
    'NG' => "Nigeria",
    'NU' => "Niue",
    'NF' => "Norfolk Island",
    'MP' => "Northern Mariana Islands",
    'NO' => "Norway",
    'OM' => "Oman",
    'PK' => "Pakistan",
    'PW' => "Palau",
    'PS' => "Palestinian Territory, Occupied",
    'PA' => "Panama",
    'PG' => "Papua New Guinea",
    'PY' => "Paraguay",
    'PE' => "Peru",
    'PH' => "Philippines",
    'PN' => "Pitcairn",
    'PL' => "Poland",
    'PT' => "Portugal",
    'PR' => "Puerto Rico",
    'QA' => "Qatar",
    'RE' => "Reunion",
    'RO' => "Romania",
    'RU' => "Russian Federation",
    'RW' => "Rwanda",
    'BL' => "Saint Barthélemy",
    'SH' => "Saint Helena",
    'KN' => "Saint Kitts and Nevis",
    'LC' => "Saint Lucia",
    'MF' => "Saint Martin",
    'PM' => "Saint Pierre and Miquelon",
    'VC' => "Saint Vincent and the Grenadines",
    'WS' => "Samoa",
    'SM' => "San Marino",
    'ST' => "Sao Tome and Principe",
    'SA' => "Saudi Arabia",
    'SN' => "Senegal",
    'RS' => "Serbia",
    'SC' => "Seychelles",
    'SL' => "Sierra Leone",
    'SG' => "Singapore",
    'SK' => "Slovakia",
    'SI' => "Slovenia",
    'SB' => "Solomon Islands",
    'SO' => "Somalia",
    'ZA' => "South Africa",
    'GS' => "South Georgia and the South Sandwich Islands",
    'ES' => "Spain",
    'LK' => "Sri Lanka",
    'SD' => "Sudan",
    'SR' => "Suriname",
    'SJ' => "Svalbard and Jan Mayen",
    'SZ' => "Swaziland",
    'SE' => "Sweden",
    'CH' => "Switzerland",
    'SY' => "Syrian Arab Republic",
    'TW' => "Taiwan, Province of China",
    'TJ' => "Tajikistan",
    'TZ' => "Tanzania, United Republic of",
    'TH' => "Thailand",
    'TL' => "Timor-Leste",
    'TG' => "Togo",
    'TK' => "Tokelau",
    'TO' => "Tonga",
    'TT' => "Trinidadand Tobago",
    'TN' => "Tunisia",
    'TR' => "Turkey",
    'TM' => "Turkmenistan",
    'TC' => "Turksand Caicos Islands",
    'TV' => "Tuvalu",
    'UG' => "Uganda",
    'UA' => "Ukraine",
    'AE' => "United Arab Emirates",
    'GB' => "United Kingdom",
    'US' => "United States",
    'UM' => "United States Minor Outlying Islands",
    'UY' => "Uruguay",
    'UZ' => "Uzbekistan",
    'VU' => "Vanuatu",
    'VE' => "Venezuela",
    'VN' => "Viet Nam",
    'VG' => "Virgin Islands, British",
    'VI' => "Virgin Islands, U.S.",
    'WF' => "Wallisand Futuna",
    'EH' => "Western Sahara",
    'YE' => "Yemen",
    'ZM' => "Zambia",
    'ZW' => "Zimbabwe",
  );
  return $code ? $country_codes[$code] : $country_codes; 
}
