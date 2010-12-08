<?php

/**
 * @file
 * Blah blah blah.
 *
 * Note: "FOVS" in comments here means "[F]lag's [O]fficial [V]iews [S]upport".
 */
 
/**
 * Implementation of hook_views_handlers().
 */
function flag_vista_views_handlers() {
  return array(
    'handlers' => array(
      'flag_vista_handler_relationship__content_by_type' => array(
        'parent' => 'views_handler_relationship',
        'file' => 'flag_vista_handler_relationships.inc',
      ),
      'flag_vista_handler_relationship__flaggings_by_flag' => array(
        'parent' => 'views_handler_relationship',
        'file' => 'flag_vista_handler_relationships.inc',
      ),
      'flag_vista_handler_relationship__flaggings_by_flag__for_user' => array(
        'parent' => 'views_handler_relationship',
        'file' => 'flag_vista_handler_relationships.inc',
      ),
      'flag_vista_handler_relationship__flaggings_by_flag_and_user' => array(
        'parent' => 'views_handler_relationship',
        'file' => 'flag_vista_handler_relationships.inc',
      ),
      'flag_vista_handler_relationship__statistics' => array(
        'parent' => 'views_handler_relationship',
        'file' => 'flag_vista_handler_relationships.inc',
      ),
      'flag_vista_handler_filter_flag' => array(
        'parent' => 'views_handler_filter_in_operator',
        'file' => 'flag_vista_handler_filter_flag.inc',
      ),
      'flag_vista_handler_field_ops' => array(
        'parent' => 'views_handler_field',
        'file' => 'flag_vista_handler_field_ops.inc',
      ),
      'flag_vista_handler_argument_fid' => array(
        'parent' => 'views_handler_argument_numeric',
        'file' => 'flag_vista_handler_argument_fid.inc',
      ),
      'flag_vista_handler_argument_flag_name' => array(
        'parent' => 'views_handler_argument',
        'file' => 'flag_vista_handler_argument_flag_name.inc',
      ),
      'flag_vista_handler_argument_content_id' => array(
        'parent' => 'views_handler_argument_numeric',
        'file' => 'flag_vista_handler_argument_content_id.inc',
      ),
    ),
  );
}

/**
 * Implementation of hook_views_data().
 */
function flag_vista_views_data() {
  $data = array();

  // ----------------------------------------------------------------------
  // The flaggings table

  $data['flag_vista_flaggings']['table']['group'] = t('Flagging');

  // Advertise as possible base table.
  $data['flag_vista_flaggings']['table']['base'] = array(
    'field' => 'fcid',
    'title' => t('Flagging'),
    'help' => t('Flaggings are records created when users flag items.'),
  );

  $data['flag_vista_flaggings']['uid'] = array(
    'title' => t('Flagging user'),
    'help' => t('The user that flagged the item.'),
    'relationship' => array(
      'base' => 'users',
      'handler' => 'views_handler_relationship',
      'label' => t('Flagging user'),
    ),
  );

  $data['flag_vista_flaggings']['fcid'] = array(
    'title' => t('FCID'),
    'help' => t('The flagging record internal ID. Could be useful with Views 3.x\'s "Grouping" feature.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
    ),
  );

  $data['flag_vista_flaggings']['fid'] = array(
    'title' => t('Flag'),
    'filter' => array(
      'help' => t('Filter the flaggings to those of certain flag(s).'),
      'handler' => 'flag_vista_handler_filter_flag',
      'numeric' => TRUE,
    ),
    'relationship' => array(
      'help' => t('The flag this flagging record belongs to.'),
      'base' => 'flag_vista_flags',
      'base field' => 'fid',
      'handler' => 'views_handler_relationship',
      'label' => t('flag'),
    ),
    'argument' => array(
      'title' => t('Flag ID (numeric)'),
      'help' => t('Filter the flaggings to those of certain flag(s).'),
      'handler' => 'flag_vista_handler_argument_fid',
      'numeric' => TRUE,
    ),
  );

  $data['flag_vista_flaggings']['timestamp'] = array(
    'title' => t('Flagging time'),
    'help' => t('The time the item was flagged by a user.'),
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_date',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_date',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_date',
    ),
  );

  // Argument for content ID, used for "Who's flagged this" views.
  $data['flag_vista_flaggings']['content_id'] = array(
    'title' => t('Content ID'),
    'help' => t('The unique ID of the item that has been flagged.'),
    'argument' => array(
      'handler' => 'flag_vista_handler_argument_content_id',
    ),
    // Should we add a 'field' entry as well? Would a 'field' be
    // useful for Views' token replacement feature?
  );

  // Sprout a relationship for every flaggable object.
  foreach (array_keys(flag_fetch_definition()) as $content_type) {
    $flag = flag_flag::factory_by_content_type($content_type);
    $info = $flag->get_views_info();

    $data['flag_vista_flaggings']['flagged_' . $content_type . '_rel'] = array(
      'title' => t('Flagged @content-type', array('@content-type' => $content_type)),
      'help' => t('The flagged @content-type.', array('@content-type' => $content_type)),
      'relationship' => array(
        'flag type' => $content_type,
        'base' => $info['views table'],
        'base field' => $info['join field'],
        'relationship field' => 'content_id',
        'handler' => 'flag_vista_handler_relationship__content_by_type',
        'label' => t('flagged @content-type', array('@content-type' => $content_type)),
      ),
    );
  }

  // Note: There's no need for a "Flagged" filter as in FOVS because our
  // "Flagging: Flag" filter can do this.

  // ----------------------------------------------------------------------
  // The flags table

  $data['flag_vista_flags']['table']['group'] = t('Flag');

  $data['flag_vista_flags']['title'] = array(
    'title' => t('Title'),
    'help' => t('The title of the flag.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  // The following relationship makes it possible to construct views like
  // #370886 (and #370343).
  $data['flag_vista_flags']['flag_flaggings_rel'] = array(
    'title' => t('Flaggings'),
    'help' => t('The flaggings that belong to this flag.'),
    'relationship' => array(
      'base' => 'flag_vista_flaggings',
      'base field' => 'fid',
      'relationship field' => 'fid',
      'handler' => 'views_handler_relationship',
      'label' => t('flag flaggings'),
    ),
  );

  $data['flag_vista_flags']['name'] = array(
    'title' => t('Machine-name'),
    'argument' => array(
      'help' => t('Filter the flags to that of a certain name.'),
      'handler' => 'flag_vista_handler_argument_flag_name',
      'name field' => 'title', // The field to display in the summary.
    ),
    // Should we add a 'field' and 'sort' entries as well? Would a 'field' be
    // useful for Views' token replacement feature?
  );

  $data['flag_vista_flags']['fid'] = array(
    'argument' => array(
      'title' => t('Flag ID (numeric)'),
      'help' => t('Filter the flags to those of a certain ID or IDs.'),
      'handler' => 'flag_vista_handler_argument_fid',
      'numeric' => TRUE,
    ),
  );

  // Note: In the future this table will also have a 'weight' column.

  // ----------------------------------------------------------------------
  // The statistics table

  $data['flag_vista_statistics']['table']['group'] = t('Flagging statistics');

  // @todo: advertize as base table?

  $data['flag_vista_statistics']['count'] = array(
    'title' => t('Counter'),
    'help' => t('The number of times an item is flagged by any user.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    // I'm not sure how useful the following is, but it's in FOVS:
    'argument' => array(
      'handler' => 'views_handler_argument_numeric',
    ),
  );

  $data['flag_vista_statistics']['fid'] = array(
    'title' => t('Flag'),
    'filter' => array(
      'help' => t('Filter the statistics to that of certain flag(s).'),
      'handler' => 'flag_vista_handler_filter_flag',
      'numeric' => TRUE,
    ),
    'relationship' => array(
      'help' => t('The flag the statistics belongs to.'),
      'base' => 'flag_vista_flags',
      'base field' => 'fid',
      'handler' => 'views_handler_relationship',
      'label' => t('flag'),
    ),
    'argument' => array(
      'title' => t('Flag ID (numeric)'),
      'help' => t('Filter the statistics to that of certain flag(s).'),
      'handler' => 'flag_vista_handler_argument_fid',
      'numeric' => TRUE,
    ),
  );

  return $data;
}

/**
 * Implementation of hook_views_data_alter().
 */
function flag_vista_views_data_alter(&$data) {

  foreach (array_keys(flag_fetch_definition()) as $flag_type) {
    $flag = flag_flag::factory_by_content_type($flag_type);
    $info = $flag->get_views_info();

    // Add the flaggings relationship.
    $data[$info['views table']]['flaggings_rel'] = array(
      'title' => t('Flaggings'),
      'help' => t('The flaggings that are set on this @flag-type.', array('@flag-type' => $flag_type)),
      'relationship' => array(
        'flag type' => $flag_type,
        'handler' => 'flag_vista_handler_relationship__flaggings_by_flag_and_user',
        'label' => t('@flag-type flaggings', array('@flag-type' => $flag_type)),
        'base' => 'flag_vista_flaggings',
        'base field' => 'content_id',
        'relationship field' => $info['join field'],
      ),
    );

    $data[$info['views table']]['flaggings_statistics_rel'] = array(
      'title' => t('Flagging statistics'),
      'help' => t('The flagging statistics that is associated with each flagged @flag-type.', array('@flag-type' => $flag_type)),
      'relationship' => array(
        'flag type' => $flag_type,
        'handler' => 'flag_vista_handler_relationship__statistics',
        'label' => t('@flag-type flagging statistics', array('@flag-type' => $flag_type)),
        'base' => 'flag_vista_statistics',
        'base field' => 'content_id',
        'relationship field' => $info['join field'],
      ),
    );

    // ...and a flag link:
    $data[$info['views table']]['flag_link'] = array(
      'title' => t('Flag link'),
      'help' => t('Provide a link to flag/unflag this @flag-type.', array('@flag-type' => $flag_type)),
      'field' => array(
        'flag type' => $flag_type,
        'handler' => 'flag_vista_handler_field_ops',
        'additional fields' => array('content_id' => array('field' => $info['join field'])),
      ),
    );

  }

  // Add a relationship for the user's flaggings.
  $data['users']['user_flaggings_rel'] = array(
    'title' => t('Flaggings by user'),
    'help' => t('The flaggings initiated by this user. These, for example, let you access the user\'s flagged items, or restrict the users to only those who flagged a certain item.'),
    'relationship' => array(
      'flag type' => NULL,
      'handler' => 'flag_vista_handler_relationship__flaggings_by_flag__for_user',
      'label' => t('flaggings by user'),
      'base' => 'flag_vista_flaggings',
      'base field' => 'uid',
      'relationship field' => 'uid',
    ),
  );

  _flag_vista_hide_fovs($data);

}


/**
 * Hides FOVS.
 *
 * It uses the "skip base" setting to do this; see http://drupal.org/node/288599
 */
function _flag_vista_hide_fovs(&$data) {
  if (!function_exists('views_fetch_base_tables')) {
    views_include('admin');
  }
  $base_tables = array_keys(views_fetch_base_tables());
  foreach (array_keys(flag_fetch_definition()) as $flag_type) {
    $flag = flag_flag::factory_by_content_type($flag_type);
    $info = $flag->get_views_info();
    $data[$info['views table']]['flag_content_rel']['skip base'] = $base_tables;
    $data[$info['views table']]['flag_count_rel']['skip base'] = $base_tables;
  }
  $data['users']['flag_user_content_rel']['skip base'] = $base_tables;
  $data['flag_content']['skip base'] = $base_tables;
  $data['flag_counts']['skip base'] = $base_tables;
}

// -----------------------------------------------------------------------
// Utility functions.

/**
 * Returns an $options array suitable for use with checkboxes or radios.
 */
function _flag_vista__flags_menu($content_type) {
  $options = array();
  foreach (flag_get_flags($content_type) as $flag) {
    $options[$flag->name] = $flag->get_title();
  }
  return $options;
}

/**
 * Returns the relationship name a handler "sits" on.
 *
 * This is a more reliable way than doing $this->options['relationship'] 
 * because the later doesn't yet exist when a handler has just been added to 
 * the view. In other words, if you need to know your relationship in your 
 * handler's settings form already (and not at a later stage (like query())) 
 * then use this function instead.
 *
 * Tecnical details: When a handler is added to a view, it's relationship is 
 * set via $view->set_item_option() (see http://drupal.org/node/313377), but 
 * this is not yet visible in $handler->options[].
 */
function _flag_vista__get_relationship_id($handler) {
  foreach (array('filter', 'relationship', 'field', 'argument', 'sort') as $type) {
    if (is_a($handler, 'views_handler_' . $type)) {
      $raw_options = $handler->view->get_item($handler->view->current_display, $type, $handler->options['id']);
      if (isset($raw_options['relationship'])) {
        return $raw_options['relationship'];
      }
      else {
        return 'none';
      }
    }
  }
}

/**
 * Returns the relationship handler we "sit" on. Or NULL if we're the base table.
 */
function _flag_vista__get_relationship($handler) {
  if (($relationship_id = _flag_vista__get_relationship_id($handler)) != 'none') {
    // We can't use $this->view->relationship[] here because it's only 
    // available when the view is being executed. It won't work, e.g., when 
    // we're on the settings form.
    return $handler->view->display_handler->get_handler('relationship', $relationship_id);
  }
}

/**
 * Returns TRUE if the flag supports anonymous flagging.
 */
function _flag_vista__is_anonymous($flag) {
  return (!$flag->global && function_exists('flag_get_sid') && (array_search(DRUPAL_ANONYMOUS_RID, $flag->roles['flag']) !== FALSE));
}

/**
 * Returns TRUE if we're running against Flag 2.x and up (as opposed to Flag 1.x).
 */
function _flag_vista__is_flag_2() {
  return function_exists('flag_get_sid');
}

