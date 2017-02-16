<div class="page-title">
  <?php
    print $node->title;
  ?>
</div>
<div class="requests">
  <div class="requests-title">
    <?php
      print t('Requests for checking of the lesson');
    ?>
  </div>
  <div class="requests-list">
    <?php
      if (!$requests) {

      }
      else {
        print '
          <div class="requests-list-header">
            <div class="header-item">' . t('#') . '</div>
            <div class="header-item">' . t('Name') . '</div>
            <div class="header-item">E-mail</div>
            <div class="header-item">' . t('Filing date') . '</div>
          </div>
          <div class="requests-list-body">
        ';
        $i = 0;
        foreach ($requests as $request) {
          $i++;
          $r_wrapper = entity_metadata_wrapper('node', $request);
          $account = $r_wrapper->author->value();
          $name = ditoolsi_profile_name($account);
          $mail = $account->mail;
          $time = $request->changed;
          $date = format_date($time);
          $lesson_id = $request->field_progress_lesson_id[LANGUAGE_NONE][0]['target_id'];

          print '
            <div class="body-item">' . $i . '</div>
            <div class="body-item">' . $name . '</div>
            <div class="body-item">' . l($mail, 'user/' . $account->uid) . '</div>
            <div class="body-item">' . $date . '</div>
            <div class="body-item">' . l(t('Check task'), "course/{$node->nid}/{$lesson_id}/{$request->nid}/review") . '</div>
          ';
        }

        print '</div>';
      }
    ?>
  </div>
</div>