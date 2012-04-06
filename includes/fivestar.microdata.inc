<?php

/**
 * @file
 * Provides integration with microdata module.
 *
 * Note that some integration takes place in hook_field_info and in field
 * formatters.
 */

/**
 * Implements hook_microdata_suggestions().
 */
function fivestar_microdata_suggestions() {
  $mappings = array();

  // Add the review mapping for Schema.org
  $mappings['fields']['fivestar']['schema.org'] = array(
    '#itemprop' => array('aggregateRating'),
    '#itemtype' => array('http://schema.org/AggregateRating'),
    'average_rating' => array(
      '#itemprop' => array('ratingValue'),
    ),
  );

  return $mappings;
}
