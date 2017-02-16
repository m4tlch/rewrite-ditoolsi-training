<div class="pupil-inform-page">
	<div class="course-inform">
		<div class="course-title"><?php print $course->title; ?></div>
		<div class="course-nid">id<?php print $course->nid; ?></div>
	</div>
	<div class="form-pupil-messages">
		<?php print(drupal_render($form_messages)); ?>			
	</div>
	<div class="form-pupil-rules">
		<?php print(drupal_render($form_rules)); ?>			
	</div>
	<div>
	</div>
</div>