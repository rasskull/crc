<?php
 $data = $row->_field_data['nid']['entity'];
 $path = image_style_url('spotlight', $data->field_image['und'][0]['uri']);
 
 if($data->field_spotlight_link['und'][0]['value']) { ?>
 <a href="<?php print $data->field_spotlight_link['und'][0]['value'] ?>"><img src="<?php print $path; ?>" /></a>
 <?php } else { ?>
 <img src="<?php print $path; ?>" />
 <?php } ?>
 