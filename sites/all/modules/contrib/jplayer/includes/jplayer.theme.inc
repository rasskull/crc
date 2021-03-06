<?php
/**
 * @file
 * Theme and preprocess functions for the jPlayer module.
 */

/**
 * Preprocess function for a player.
 */
function template_preprocess_jplayer(&$vars) {
  $vars['mode'] = $vars['settings']['mode'];
  
  $player = jplayer_sort_files($vars['items'], $vars['player_id'], $vars['mode']);
  
  $vars['playlist'] = theme('jplayer_item_list', array('items' => $player['playlist']));
  $vars['type'] = $player['type'];
  
  // Add player settings
  $player = array(
    'jplayerInstances' => array(
      $vars['player_id'] => array(
        'files' => $player['files'],
        'solution' => $vars['settings']['solution'],
        'supplied' => $player['extensions'],
        'preload' => $vars['settings']['preload'],
        'volume' => $vars['settings']['volume'] / 100,
        'muted' => (boolean)$vars['settings']['muted'],
        'autoplay' => (boolean)$vars['settings']['autoplay'],
        'repeat' => $vars['settings']['repeat'],
        'backgroundColor' => $vars['settings']['backgroundColor'],
      ),
    ),
  );
  drupal_add_js($player, 'setting');
  
  $vars['dynamic'] = jplayer_add_core_js();
}

/**
 * Modified version of theme_item_list().
 */
function theme_jplayer_item_list($variables) {
  $items = $variables['items'];
  $title = $variables['title'];
  $type = $variables['type'];
  $attributes = $variables['attributes'];

  $output = '';
  if (isset($title)) {
    $output .= '<h3>' . $title . '</h3>';
  }

  if (!empty($items)) {
    $output .= "<$type" . drupal_attributes($attributes) . '>';
    $num_items = count($items);
    foreach ($items as $i => $item) {
      $attributes = array();
      $children = array();
      if (is_array($item)) {
        foreach ($item as $key => $value) {
          if ($key == 'data') {
            $data = $value;
          }
          elseif ($key == 'children') {
            $children = $value;
          }
          else {
            $attributes[$key] = $value;
          }
        }
      }
      else {
        $data = $item;
      }
      if (count($children) > 0) {
        // Render nested list.
        $data .= theme_item_list(array('items' => $children, 'title' => NULL, 'type' => $type, 'attributes' => $attributes));
      }
      if ($i == 0) {
        $attributes['class'][] = 'first jp-playlist-first';
      }
      if ($i == $num_items - 1) {
        $attributes['class'][] = 'last jp-playlist-last';
      }
      $output .= '<li' . drupal_attributes($attributes) . '>' . $data . "</li>\n";
    }
    $output .= "</$type>";
  }
  return $output;
}

/**
 * Filters an array of files into groups ready for jPlayer.
 */
function jplayer_sort_files($raw_files = array(), $player_id, $type = 'single') {
  $video_extensions = array('m4v', 'mp4', 'ogv', 'webmv');
  $audio_extensions = array('mp3', 'm4a', 'oga', 'webma', 'wav');
  $poster_extensions = array('jpg', 'jpeg', 'png', 'gif');
  
  $poster = NULL;
  $videos = FALSE;
  $audio = FALSE;
  $media = array();
  
  $files = array();
  $extensions = array();
  $playlist = array();
  
  // Look through all the files provided and see what we have
  foreach($raw_files as $delta => $item) {
    // Get file URL
    if (!isset($item['url'])) {
      $item['url'] = file_create_url($item['uri']);
    }
    
    // Get file extension
    if (!isset($item['ext'])) {
      $fileinfo = pathinfo($item['url']);
      $item['ext'] = $fileinfo['extension'];
    }
    
    // Get file label
    if (!isset($item['label'])) {
      if (empty($item['description'])) {
        $item['label'] = $item['filename'];
      }
      else {
        $item['label'] = $item['description'];
      }
    }
    
    // Add file into correct group
    if (in_array($item['ext'], $video_extensions)) {
      $videos = TRUE;
      $item['type'] = 'audio';
    }
    else if (in_array($item['ext'], $audio_extensions)) {
      $audio = TRUE;
      $item['type'] = 'video';
    }
    else if (in_array($item['ext'], $poster_extensions)) {
      $poster = $item['url'];
      $item['type'] = 'poster';
    }
    
    $media[] = $item;
  }
  
  $num = 0;
  foreach ($media as $file) {
    if ($videos == TRUE && $type != 'playlist') {
      if ($file['type'] == 'audio') {
        unset($file);
      }
      $player_type = 'video';
    }
    else if ($videos == TRUE && $type == 'playlist') {
      $player_type = 'video';
    }
    else {
      $player_type = 'audio';
    }
    
    if (isset($file) && $file['type'] == 'audio') {
      $files[][$file['ext']] = $file['url'];
      $extensions[] = $file['ext'];
      $playlist[] = l($file['label'], $file['url'], array('attributes' => array('id' => $player_id . '_item_' . $num, 'tabindex' => 1)));
      $num++;
    }
    else if (isset($file) && $file['type'] == 'video') {
      $files[][$file['ext']] = $file['url'];
      if ($poster != NULL) {
        $files[]['poster'] = $poster;
      }
      $extensions[] = $file['ext'];
      $playlist[] = l($file['label'], $file['url'], array('attributes' => array('id' => $player_id . '_item_' . $num, 'tabindex' => 1)));
      $num++;
    }
  }
  
  if ($type == 'single') {
    $item = array_shift($playlist);
    $playlist = array();
    $playlist[] = $item;
  }
  
  return array('files' => $files, 'extensions' => implode(',', $extensions), 'playlist' => $playlist, 'type' => $player_type);
}

/**
 * Preprocess function for jplayer.tpl.php when displaying a view as a playlist.
 */
function template_preprocess_jplayer_view_playlist(&$vars) {
  $view = $vars['view'];
  $vars['settings'] = $view->style_plugin->options;
  $vars['mode'] = 'playlist';
  $vars['player_id'] = 'jplayer-view-' . str_replace('_', '-', check_plain($view->name));
  
  $player = jplayer_sort_files($vars['items'], $vars['player_id'], $vars['mode']);
  
  $vars['playlist'] = theme('jplayer_item_list', array('items' => $player['playlist']));
  $vars['type'] = $player['type'];
  
  // Add player settings
  $player = array(
    'jplayerInstances' => array(
      $vars['player_id'] => array(
        'files' => $player['files'],
        'solution' => $vars['settings']['solution'],
        'supplied' => $player['extensions'],
        'preload' => $vars['settings']['preload'],
        'volume' => $vars['settings']['volume'] / 100,
        'muted' => (boolean)$vars['settings']['muted'],
        'autoplay' => (boolean)$vars['settings']['autoplay'],
        'repeat' => $vars['settings']['repeat'],
        'backgroundColor' => $vars['settings']['backgroundColor'],
      ),
    ),
  );
  drupal_add_js($player, 'setting');
  
  $vars['dynamic'] = jplayer_add_core_js();
}