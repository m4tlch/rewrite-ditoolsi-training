<div class="task-history">
  <div class="task-answer">
    <label><?php print t('Answer:') ?></label>
    <div><?php print $answer ?></div>
  </div>
  <?php
    if (count($comments)):
  ?>
    <div class="task-comments">
      <label><?php print t('Comments:') ?></label>
      <?php
        foreach ($comments as $comment) {
          print $comment;
        }
      ?>
    </div>
  <?php
    endif;
    if (count($files)):
  ?>
    <div class="task-files">
      <label><?php print t('Files:') ?></label>
      <?php
        foreach ($files as $file) {
          print $file;
        }
      ?>
    </div>
    <div class="task-links">
      <label><?php print t('Links:') ?></label>
      <?php
        foreach ($links as $link) {
          print $link;
        }
      ?>
    </div>    
  <?php
    endif;
  ?>
</div>