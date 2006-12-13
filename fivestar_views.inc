<?php
// $Id$

function fivestar_views_tables() {
  $tables["fivestar_results"] = array(
    "name" => "votingapi_cache", 
    "provider" => "votingapi",
    "join" => array(
      "left" => array(
        "table" => "node",
        "field" => "nid"
      ), 
      "right" => array(
        "field" => "content_id"
      ),
      "extra" => array(
        'content_type' => 'node',
        'value_type' => 'percent',
        'tag' => 'vote',
        'function' => 'average',
      ),
    ),
    "fields" => array(
      "value" => array(
        'name' => t("Fivestar: Vote results"),
        'sortable' => true,
        'handler' => 'fivestar_views_value_display_handler',
      ),
    ),
    "sorts" => array(
      "value" => array('name' => "Fivestar: Vote results")
    ),
    "filters" => array(
      "value" => array(
        'name' => t("Fivestar: Average vote"),
        'operator' => views_handler_operator_gtlt(),
        'handler' => 'fivestar_views_nullable_field_handler',
        'help' => t("Filter by the value of a vote."),
      ),
    ),
  );
  
  $tables["fivestar_votes"] = array(
    "name" => "votingapi_vote", 
    "provider" => "votingapi",
    "join" => array(
      "left" => array(
        "table" => "node",
        "field" => "nid"
      ), 
      "right" => array(
        "field" => "content_id"
      ),
      "extra" => array(
        'content_type' => 'node',
        'value_type' => 'percent',
        'tag' => 'vote',
      ),
    ),
    "fields" => array(
      "value" => array(
        'name' => t("Fivestar: Individual vote"),
        'sortable' => true,
        'handler' => 'fivestar_views_value_display_handler',
        'sortable' => true,
      ),
      "timestamp" => array(
        'name' => t("Fivestar: Individual vote timestamp"),
        'sortable' => true,
        'handler' => 
          array(
            "views_handler_field_date_small"=>"As Short Date", 
            "views_handler_field_date"=>"As Medium Date", 
            "views_handler_field_date_large"=>"As Long Date", 
            "views_handler_field_since" => "As Time Ago"
          ),
        'sortable' => true,
        'addlfields' => array('value_type', 'tag', 'uid', 'content_type', 'content_id'),
        'help' => "Display the time the vote was cast.",
      ),
      "uid" => array(
        'name' => t("Fivestar: Individual voter"),
        'handler' => 'fivestar_views_handler_field_username', 
        'sortable' => true, 
        'uid' => "uid", 
        'addlfields' => array('uid', 'content_type', 'content_id'),
      ),
    ),
    "sorts" => array(
      "value" => array('name' => "Fivestar: Individual vote")
    ),
    "filters" => array(
      "value" => array(
        'name' => t("Fivestar: Individual vote"),
        'operator' => views_handler_operator_gtlt(),
        'handler' => 'fivestar_views_nullable_field_handler',
        'help' => t("Filter by the value of a vote."),
      ),
      "currentuidtouched" => array(
        'field' => 'uid',
        'name' => "Fivestar: Current user has voted",
        'operator' => array('=' => "voted on by"),
        'list' => "views_handler_filter_usercurrent",
        'list-type' => 'select',
        'handler' => "fivestar_views_handler_filter_uid_has_voted",
        'help' => "This allows you to filter by whether or not the logged in user has voted on the node.",
      ),
    ),
  );

  return $tables;
}

function fivestar_views_handler_filter_uid_has_voted($op, $filter, $filterinfo, &$query) {
  $query->add_where("votingapi_vote.uid = %d", $filter['value']);
}

function fivestar_views_handler_filter_uid_voted($op, $filter, $filterinfo, &$query) {
  $query->add_where("votingapi_vote.uid $filter[operator] %d OR votingapi_vote.uid IS NULL", $filter['value']);
}

function fivestar_views_nullable_field_handler($op, $filter, $filterinfo, &$query) {
  $tn = $query->add_table($filterinfo['table']);
  $tname = $filter['field'];
  $fvalue = $filter['value'];
  $fop = $filter['operator'];
  $query->add_where("$tname $fop '$fvalue' OR $tname IS NULL");
}

function fivestar_views_value_display_handler($op, $filter, $value, &$query) {
  if (!isset($value)) {
    return t('No votes');
  }
  else {
    return theme('fivestar_static', $value);
  }
}

function fivestar_views_handler_field_username($fieldinfo, $fielddata, $value, $data) {
  $uidfield = $fielddata['tablename'] . "_"  . $fieldinfo['uid'];
  $user = user_load(array('uid' => $data->$uidfield));
  return theme('username', $user);
}