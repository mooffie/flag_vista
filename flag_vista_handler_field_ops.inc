<?php
// $Id: flag_handler_field_ops.inc,v 1.1.2.5.2.3 2010-01-08 04:43:17 quicksketch Exp $

/**
 * @file
 * Contains the flag Ops field handler.
 */

// We name it _single because in the future we may have one that handles multiple flags.
class flag_vista_handler_field_ops_single extends views_handler_field {

  function option_definition() {
    $options = parent::option_definition();
    $options['flag'] = array('default' => NULL);
    $options['link_type'] = array('default' => '');
    return $options;
  }

  function options_form(&$form, &$form_state) {

    $content_type = $this->definition['flag type'];

    $form['flag'] = array(
      '#type' => 'radios',
      '#title' => t('Flag'),
      '#options' => _flag_vista__flags_menu($content_type),
      '#default_value' => $this->options['flag'],
      '#description' => t('Select the flag whose link to display.'),
      '#required' => TRUE,
    );

    $form['link_type'] = array(
      '#type' => 'radios',
      '#title' => t('Link type'),
      '#options' => array('' => t('Use flag link settings')) + _flag_link_type_options(),
      '#default_value' => $this->options['link_type'],
    );

    parent::options_form($form, $form_state);
  }

  function query() {
    $this->ensure_my_table();
    $this->add_additional_fields();
  }

  function get_flag() {
    return flag_get_flag($this->options['flag']);
  }

  /**
   * Find out the status of each item (flagged/unflaggd), and whether it's flaggable.
   */
  function pre_render($values) {
    global $user;
    $flag = $this->get_flag();
    $is_anonymous = _flag_vista__is_anonymous($flag);

    $actions = array();
    foreach ($values as $row) {
      if ($id = $row->{$this->aliases['content_id']}) { // Skip NULLs.
        $actions[$id] = 'flag';
      }
    }

    $query = "SELECT content_id FROM {flag_content} WHERE fid = %d AND content_id IN (" . implode(', ', array_keys($actions)) . ") AND uid = %d";
    if ($is_anonymous) {
      $query .= ' AND sid = %d';
    }
    $result = db_query($query, $flag->fid, $flag->global ? 0 : $user->uid, $is_anonymous ? flag_get_sid() : 0);

    while ($row = db_fetch_object($result)) {
      $actions[$row->content_id] = 'unflag';
    }

    $this->flag_actions = $actions;
    if (_flag_vista__is_flag_2()) {
      $this->flag_applies = $actions ? $flag->access_multiple($actions) : array();
    }
    else {
      $this->flag_applies = $actions ? $flag->applies_to_content_id_array(array_keys($actions)) : array();
    }
  }

  function render($values) {
    $flag = $this->get_flag();
    $content_id = $values->{$this->aliases['content_id']};

    if (empty($this->flag_applies[$content_id])) {
      // Flag does not apply to this content.
      return;
    }

    if (!empty($this->options['link_type'])) {
      $flag->link_type = $this->options['link_type'];
    }
    return $flag->theme($this->flag_actions[$content_id], $content_id);
  }

}

