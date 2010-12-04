<?php


class flag_vista_handler_filter_flag extends views_handler_filter_in_operator {

  /**
   * Figures out the content type (aka "flag type") our relationship is 
   * restricted to.
   *
   * Returns NULL if we aren't restricted.
   */
  function get_content_type() {
    if (($relationship = _flag_vista__get_relationship($this)) && isset($relationship->definition['flag type'])) {
      return $relationship->definition['flag type'];
    }
  }

  function get_value_options() {
    if (!isset($this->value_options)) {
      $options = array();
      foreach (flag_get_flags($this->get_content_type()) as $name => $flag) {
        $options[$name] = $flag->get_title();
      }
      $this->value_options = $options;
      $this->value_title = t('Flags');
    }
  }

  /**
   * To make it possible to export/import views between systems, we store flag 
   * names, not numeric IDs, in the view. But we need to convert them to 
   * numeric IDs before executing the query.
   */
  function query() {
    if (!empty($this->value)) {
      $fids = array();
      foreach ($this->value as $flag_name) {
        if (($flag = flag_get_flag($flag_name))) {
          $fids[$flag->fid] = $flag->fid;
        }
        // @todo: use the "-555" trick?
      }
      $this->value = $fids;
    }
    return parent::query();
  }

}
