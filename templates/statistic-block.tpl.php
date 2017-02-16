<table class="statistic-table">
  <thead>
  <th><?php print t('My pace of learning:'); ?></th>
  <th><?php print t('Optimal rates:'); ?></th>
  <th><?php print t('Forecast results:'); ?></th>
  </thead>
  <tbody>
  <tr>
    <td>
      <div class="grey-text"><?php print t('The average speed of the lesson:') ?></div>
      <div>
        <?php
        if ($middle_time > 2592000) {
          $month = floor($middle_time / 2592000) . t(' m. ');
        } else {
          $month = NULL;
        }

        if ($middle_time > 86400) {
          $day = ($middle_time / 86400) % 30 . t(' d. ');
        } else {
          $day = NULL;
        }

        if ($middle_time > 3600) {
          $hour = ($middle_time / 3600) % 24 . t(' h. ');
        } else {
          $hour = NULL;
        }

        if ($middle_time > 60) {
          $min = ($middle_time / 60) % 60 . t(' min. ');
        } else {
          $min = NULL;
        }

        if ($middle_time > 60) {
          $sec = $middle_time % 60 . t(' sec. ');
        } else {
          $sec = NULL;
        }

        $transmission_rate = $month . $day . $hour . $min . $sec;
        print $transmission_rate;
        ?>
      </div>
    </td>
    <td><?php print !empty($optimal_temp) ? $optimal_temp->field_optimal_rate_learning_value . t(' lesson of the day') : ''; ?></td>
    <td>
      <div class="grey-text"><?php print t('At this rate, you will take the course for:') ?> </div>
      <div>
        <?php

        if ($forecast_time>2592000) {
          $f_month = floor( $forecast_time / 2592000 ) . t(' m. ');
        } else {
          $f_month = NULL;
        }
        if ($forecast_time>86400) {
          $f_day = ( $forecast_time / 86400 ) % 30 . t(' d. ');
        } else {
          $f_day = NULL;
        }
        if ($forecast_time>3600) {
          $f_hour = ( $forecast_time / 3600 ) % 24 . t(' h. ');
        } else {
          $f_hour = NULL;
        }
        if ($forecast_time>60) {
          $f_min = ( $forecast_time / 60 ) % 60 . t(' min. ');
        } else {
          $f_min = NULL;
        }
        if ($forecast_time>60) {
          $f_sec = $forecast_time % 60 . t(' sec. ');
        } else {
          $f_sec = NULL;
        }
        $forecast_rate = $f_month . $f_day . $f_hour . $f_min . $f_sec;
        print $forecast_rate;
        ?></div>
    </td>
  </tr>
  <tr>
    <td>
      <div class="grey-text"><?php print t('On average, you get a lesson:') ?></div>
      <div><?php print '0'; ?></div>
    </td>
    <td><?php print t('0 score'); ?></td>
    <td>
      <div class="grey-text"><?php print t('Completing the course you will gain:'); ?></div>
      <div><?php print '0'; ?></div>
    </td>
  </tr>
  </tbody>
</table>
<div class="c_content">
  <span class="c_value"><?php	print t('Passed lessons - ') . $count_time; ?></span>
  <span class="c_value">(<?php print $procent_finished . t('% of course'); ?>)</span>
</div>