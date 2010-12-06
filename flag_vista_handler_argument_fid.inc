<?php

class flag_vista_handler_argument_fid extends views_handler_argument_numeric {

  /**
   * Override the behavior of summary_name(). Get the user friendly version
   * of the flag ID.
   */
  function summary_name($data) {
    return $this->flag_title($data->{$this->name_alias});
  }

  /**
   * Override the behavior of title_query() (and thus title()). Get the user 
   * friendly version of the flag ID. This method is used for "%1"-style 
   * rpelacements.
   */
  function title_query() {
    $titles = array();
    foreach ($this->value as $fid) {
      $titles[] = $this->flag_title($fid);
    }
    return $titles;
  }

  function flag_title($fid) {
    if (($flag = flag_get_flag(NULL, $fid))) {
      return $flag->get_title();
    }
    else {
      return t('Invalid flag ID @flag-id', array('@flag-id' => $fid));
    }
  }

}

