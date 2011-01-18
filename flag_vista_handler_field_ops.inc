<?php

/**
 * @file
 * Contains the flag Ops field handler.
 */

class flag_vista_handler_field_ops extends views_handler_field {

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
    if (!($flag = $this->get_flag())) {
      return; // render() will take care of printing an error message.
    }
    $is_anonymous = _flag_vista__is_anonymous($flag);

    $actions = array();
    foreach ($values as $row) {
      if ($id = $row->{$this->aliases['content_id']}) { // Skip NULLs.
        $actions[$id] = 'flag';
      }
    }

    // No items to inspect. We exit to prevent issuing an invalid SQL query ("... content_id IN ()").
    if (!$actions) {
      $this->flag_actions = array();
      $this->flag_applies = array();
      return;
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
      $this->flag_applies = $flag->access_multiple($actions);
    }
    else {
      $this->flag_applies = $flag->applies_to_content_id_array(array_keys($actions));
    }
  }

  function render($values) {
    if (!($flag = $this->get_flag())) {
      return t('Missing flag "@flag-name".', array('@flag-name' => $this->options['flag']));
    }
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

