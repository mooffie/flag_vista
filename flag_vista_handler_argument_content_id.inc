<?php

/**
 * Currently, this handler does nothing. Perhaps we will extend it someday to 
 * do something useful. Or delete it altogether.
 *
 * - FOVS uses a similar handler to provide the "%1" title, but we can't do 
 *   this (see http://drupal.org/node/991472#vista-content-it for reason).
 *
 * - Theres a feature request for "listing all users who haven't bookmarked a 
 *   certain node"; see http://drupal.org/node/479390
 */
class flag_vista_handler_argument_content_id extends views_handler_argument_numeric {
}

