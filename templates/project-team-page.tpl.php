<?php
$teacher_fio = $teacher->field_first_name[LANGUAGE_NONE][0]['value'] . ' ' . $teacher->field_last_name[LANGUAGE_NONE][0]['value'];
$query = new EntityFieldQuery();
$query
  ->entityCondition('entity_type', 'node')
  ->entityCondition('bundle', 'project_team')
  ->propertyCondition('status', NODE_PUBLISHED)
  ->fieldCondition('field_project_team_course', 'target_id', $node->nid);

$result = $query->execute();

if (isset($result['node'])) {
  $nid = key($result['node']);
  $project_team = node_load($nid);
}

if (!empty($project_team->field_project_team_course_info[LANGUAGE_NONE][0]['value'])):
?>
<div class="course-block">
  <?php
    print theme('ditoolsi_training_course_block', array(
      'node' => $node,
      'picture' => $wrapper->field_course_image->value(),
      'type' => $wrapper->field_course_type->value(),
      'specialization' => $wrapper->field_course_specialization->value(),
      'time_completion' => $wrapper->field_course_time_completion->value(),
      'sn_group' => $wrapper->field_course_sn_group->value(),
      'wrapper' => $wrapper,
      'pupil' => NULL,
    ));
  ?>
</div>
<div class="course-content">
  <div class="course-title">
    <?php
      print $node->title;
    ?>
  </div>

  <div class="course-body">
    <?php
      print $description;
    ?>
  </div>
</div>
<?php endif; ?>
<div class="team-content">
  <div class="description">
  <?php
  $default = t('<p>' . 'To access the curator, go to My messages, select the contact list, click a message, after writing a message sure to press Send.' . '</p>
  <p>' . 'E-mail is the curator, so to get a quick response. You can specify the contacts to which you want to get the answer. This will further speed up the necessary information.' . '</p>
  <p>' . 'Have to give additional contact the curator to svzyvatsya with him in a more convenient communication channel, if they are not represented before.' . '</p>
  <p>' . 'If you experience technical difficulties with the work on the website, please use the actions that can help solve the problem:' . '</p>
  <p>' . '1 Clear cookies and cache in the browser that are running. After that, go to the office and press ctrl + R starts to work in the office.' . '</p>
  <p>' . '2 Try to open a personal account from another browser (possibly in the browser settings you do not allow to work comfortably or not display some elements, as we do not always have time to fix the settings after updating the browser itself (not always update the browser to work correctly))' . '</p>
  <p>' . 'If these suggestions do not help please contact us at the contacts listed below. Please indicate in the report:' . '</p>
  <p>' . '1 Your name' . '</p>
  <p>' . '2 Course name or id (each course is unique)' . '</p>
  <p>' . '3 Login login (check that)' . '</p>
  <p>' . '4 Password (otherwise we reset it to 1, because we need to get into your account)' . '</p>
  <p>' . '5 Describe the problem step by step, so that we can immediately understand the reason) Poor description leads to additional inquiries about the problem, help to avoid this.' . '</p>
  <p>' . 'Please note that questions about the course materials (videos, presentations, assignments) is only responsible curator!' . '</p>');

    if (!empty($project_team)) {
      if ($body = field_get_items('node', $project_team, 'body')) {
        print check_markup($body[0]['value'], $body[0]['format']);
      } else {
        print $default;
      }

      if (($item = menu_get_item("node/{$nid}/edit")) && $item['access']) {
        print l('Редактировать страницу', "node/{$nid}/edit", array(
          'query' => array(
            'cid' => $node->nid,
          ),
          'attributes' => array(
            'class' => array(
              'btn', 'dit-submit', 'dit-submit-blue',
            ),
          ),
        ));
      }
    } else {
      print $default;

      if (($item = menu_get_item('node/add/project-team')) && $item['access']) {
        print l('Редактировать страницу', 'node/add/project-team', array(
          'query' => array(
            'cid' => $node->nid,
          ),
          'attributes' => array(
            'class' => array(
              'btn', 'dit-submit', 'dit-submit-blue',
            ),
          ),
        ));
      }
    }
  ?>
</div>
