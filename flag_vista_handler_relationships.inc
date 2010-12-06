<?php
/**
 * @file
 * Implements various relationships.
 *
 * The inheritance hierarchy:
 *
 *   views_handler_relationship
 *     ~content_by_type                    (e.g.: "Flagging: Flagged node")
 *     ~flaggings_by_flag
 *       ~statistics                       (e.g.: "Node: Flagging statistics")
 *       ~flaggings_by_flag_and_user       (e.g.: "Node: Flaggings")
 *       ~flaggings_by_flag__for_user      (e.g.: "User: Flaggings by user")
 */

/**
 * A relationship that links to a "content" table (e.g., 'node', 'users', 
 * 'comments').
 *
 * The table we're going out of is usually the flaggings table (but could also 
 * be the flagging statistics).
 */
class flag_vista_handler_relationship__content_by_type extends views_handler_relationship {

  function option_definition() {
    $options = parent::option_definition();
    // By default make it an inner join.
    $options['required'] = array('default' => 1);
    return $options;
  }

  /**
   * Called to implement a relationship in a query.
   *
   * We're linking only flagging records that are of compatible content-type; 
   * i.e., we don't want to link user flaggings to the node table.
   */
  function query() {
    $this->ensure_my_table(); // We need table_alias.
    $this->definition['extra'][] = array(
      'table' => $this->table_alias,
      'field' => 'content_type',
      'value' => $this->definition['flag type'],
    );
    return parent::query();
  }

}

/**
 * A relationship that links to the flaggings table. Or to the flagging 
 * statistics table. (In other words, to any table that has 'fid' and/or 
 * 'content_type').
 *
 * (And see the derived classes.)
 */
class flag_vista_handler_relationship__flaggings_by_flag extends views_handler_relationship {

  /**
   * Notify the admin of any missing flag(s).
   *
   * When importing views, or when deleting flags, a relationship may end up 
   * referring to non existent flags. This won't cause the view to crash, but 
   * the admin would probably appreciate being informed of that.
   */
  function validate() {
    $errors = array();
    foreach ($this->options['flags'] as $flag_name) {
      if (!flag_get_flag($flag_name)) {
        $errors[] = t('This view is looking for a flag by the name "@flag-name", but there is no such flag. Perhaps it was deleted. Please update the relationship "@relationship-name" in this view to use existing flags only.', array(
          '@flag-name' => $flag_name,
          '@relationship-name' => $this->ui_name(),
        ));
      }
    }
    return $errors;
  }

  function option_definition() {
    $options = parent::option_definition();
    $options['flags'] = array('default' => array());
    // By default make it an inner join.
    $options['required'] = array('default' => 1);
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['label']['#description'] .= ' '. t('The name(s) of the selected flag(s) makes a good label.');
    $form['required']['#description'] = t("The effect of 'require' is that only flagged items will be listed. Leave unchecked to list all items; or, in combination with the <em>Flag</em> filter, <a href='@unflagged-url'>to limit the results to specifically unflagged items</a>.", array('@unflagged-url' => 'http://drupal.org/node/299335'));

    $content_type = $this->definition['flag type'];

    $form['flags'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Flags'),
      '#options' => _flag_vista__flags_menu($content_type),
      '#default_value' => $this->options['flags'],
      '#description' => t("Select the flags whose flaggings to include. You'll usually select only one. Selecting none is like selecting all the flags. Selecting multiple flags (or none) usually makes sense in combination with a <em>Flag</em> exposed filter, or with Views 3.x's <em>Grouping</em> feature. NOTE: The correct way to list items that are flagged by two (or more) flags is to use two (or more) 'require'd relationships."),
    );

    if (!$form['flags']['#options']) {
      $form = array(
        'error' => array(
          '#value' => '<p class="error form-item">' . t('No %type flags exist. You must first <a href="@create-url">create a %type flag</a> before being able to use this relationship type.', array('%type' => $content_type, '@create-url' => url('admin/build/flags'))) . '</p>',
        ),
      );
      $form_state['no flags exist'] = TRUE;
    }
  }

  // Adds the flag name (the first, if there are multiple) to the admin UI.
  function admin_summary() {
    $output = '';
    if ($this->options['flags']) {
      $first = reset($this->options['flags']);
      $output = (flag_get_flag($first) ? flag_get_flag($first)->get_title() : $first);
      if (count($this->options['flags']) > 1) {
        $output .= ', ...';
      }
    }
    return $output;
  }

  function options_validate($form, &$form_state) {
    if (!empty($form_state['no flags exist'])) {
      form_error($form, t('You must first create a flag'));
    }
  }

  function options_submit(&$form, &$form_state) {
    // Convert FAPI's checkboxes value to a nice plain array.
    $form_state['values']['options']['flags'] = array_keys(array_filter($form_state['values']['options']['flags']));
    return parent::options_submit($form, $form_state);
  }

  function get_flag_ids() {
    $fids = array();
    foreach ($this->options['flags'] as $flag_name) {
      $fids[] = flag_get_flag($flag_name)->fid;
    }
    return $fids;
  }

  /**
   * Called to implement a relationship in a query.
   */
  function query() {

    //
    // Filter by flags.
    //
    if ($this->options['flags']) {
      $fids = $this->get_flag_ids();
      $this->definition['extra'][] = array(
        'field' => 'fid',
        // In case all of the selected flags are later deleted we don't want 
        // "IN ()" to be generated because it's invalid SQL. So we use some 
        // dummy value. "-555", as opposed to "-1", hints that this isn't a 
        // bug.
        'value' => $fids ? $fids : -555,
        'numeric' => TRUE,
      );
    }
    else {
      // We're asked to show flaggings of any flag. But we limit them to only 
      // ones compatible with our content type.
      if (isset($this->definition['flag type'])) {
        $this->definition['extra'][] = array(
          'field' => 'content_type',
          'value' => $this->definition['flag type'],
        );
      }
    }

    return parent::query();
  }

}

/**
 * A relationship that links to the flagging statistics table.
 *
 * This class exists for one reason only: to circumvent a "bug" in Flag.
 */
class flag_vista_handler_relationship__statistics extends flag_vista_handler_relationship__flaggings_by_flag {

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['flags']['#description'] = str_replace('flaggings', 'flagging statistics', $form['flags']['#description']);
  }

  function query() {
    if (!empty($this->options['required'])) {
      // Unfortunately, we may have zeros in our table, so having
      // parent::query() do INNER JOIN doesn't suffice. We need to filter these
      // zeros out.
      // @todo Make sure zero records aren't written in the first place, and
      // remove this code.
      $this->definition['extra'][] = array(
        'field' => 'count',
        'operator' => '>',
        'value' => '0',
        'numeric' => TRUE,
      );
    }
    return parent::query();
  }
}

/**
 * A relationship that links to the flaggings table.
 *
 * The table we're going out of is the users table. This class exists for one 
 * reason only: to tweak the help strings.
 */
class flag_vista_handler_relationship__flaggings_by_flag__for_user extends flag_vista_handler_relationship__flaggings_by_flag {

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['required']['#description'] = t("The effect of 'require' is that only flagging users will be listed.");
    // Remove the sentence about selecting items flagged by multiple flages (yes, it's ugly this way):
    $form['flags']['#description'] = preg_replace('/NOTE: .*./', '', $form['flags']['#description']);
  }
}

/**
 * A relationship that links to the flaggings table.
 *
 * This class extends its parent class by adding the ability to filter the 
 * flaggins to only those made by the "Current user".
 */
class flag_vista_handler_relationship__flaggings_by_flag_and_user extends flag_vista_handler_relationship__flaggings_by_flag {

  function option_definition() {
    $options = parent::option_definition();
    $options['user_scope'] = array('default' => 'current');
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $form['user_scope'] = array(
      '#type' => 'radios',
      '#title' => t('By'),
      '#options' => array('current' => t('Current user'), 'any' => t('Any user')),
      '#default_value' => $this->options['user_scope'],
    );

    if (module_exists('session_api')) {
      $form['session_warning'] = array(
        '#value' => '<p class="warning form-item">' . t('<strong>Warning</strong>: Adding this relationship for any flag that contains <strong>anonymous flagging access</strong> will disable page caching for anonymous users when this view is executed. It is recommended to create a dedicated page for views containing anonymous user data.', array('@create-url' => url('admin/build/flags'))) . '</p>',
      );
    }
  }

  /**
   * Counts, separately, the global and non-global flags.
   */
  function count_flags() {
    $counter = array(
      'globals' => 0,
      'non_globals' => 0,
      'anonymous' => 0,
    );
    $flag_names = $this->options['flags'] ? $this->options['flags'] : array_keys(flag_get_flags($this->definition['flag type']));
    foreach ($flag_names as $name) {
      if (($flag = flag_get_flag($name))) {
        if ($flag->global) {
          $counter['globals']++;
        }
        else {
          $counter['non_globals']++;
        }
        if (_flag_vista__is_anonymous($flag)) {
          $counter['anonymous']++;
        }
      }
    }
    return $counter;
  }

  /**
   * Called to implement a relationship in a query.
   */
  function query() {

    //
    // Filter by user.
    //
    if ($this->options['user_scope'] == 'current') {
      $have = $this->count_flags();

      if ($have['non_globals']) {
        $uid = '***CURRENT_USER***';
        if ($have['globals']) {
          // If there are global flags, we also look for the #0 user.
          $uid = array('***CURRENT_USER***', 0);
        }
        $this->definition['extra'][] = array(
          'field' => 'uid',
          'value' => $uid,
          'numeric' => TRUE,
        );

        // Handle anonymous flaggings.
        if ($have['anonymous']) {
          if (!$have['globals']) {
            // Disable page caching for anonymous users.
            $GLOBALS['conf']['cache'] = CACHE_DISABLED;

            // Add in the SID from Session API for anonymous users.
            $this->definition['extra'][] = array(
              'field' => 'sid',
              'value' => flag_get_sid(),
              'numeric' => TRUE,
            );
          }
        }
      }
    }

    return parent::query();
  }

}

