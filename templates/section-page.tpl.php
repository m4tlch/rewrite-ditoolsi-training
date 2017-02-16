<?php

  $files    = $wrapper->field_section_file->value();
?>
<div class="info-label">
  <?php
    print t('Information about section');
  ?>
</div>
<div class="section-info" id="section">
  <div class="lesson-title">
    <?php
      print $node->title;
    ?>
  </div>

  <div class="lesson-body">
    <?php
      $body = $wrapper->body->value();
      print check_markup($body['value'], $body['format']);
    ?>
  </div>
  <div class="section-files">
    <?php
      foreach ($files as $value) {
        print l($value['filename'], file_create_url($value['uri']));
      }
    ?>
  </div>
</div>
