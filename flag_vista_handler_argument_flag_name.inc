<?php

class flag_vista_handler_argument_flag_name extends views_handler_argument {

  /**
   * Override the behavior of title(). Get the user friendly version of the
   * flag name. This method is used for "%1"-style rpelacements. (Summary titles
   * are already handled by the 'name field' on the argument definition.)
   */
  function title() {
    return $this->flag_title($this->argument);
  }

  function flag_title($name) {
    if (($flag = flag_get_flag($name))) {
      return $flag->get_title();
    }
    else {
      return t('Invalid flag name @flag-name', array('@flag-name' => $name));
    }
  }

}

