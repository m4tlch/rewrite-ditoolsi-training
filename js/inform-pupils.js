(function ($) {

  Drupal.behaviors.ditoolsiTrainingDayPicker = {
    attach: function(context, setting) {
      $('body').once('ditoolsiTrainingDayPicker', function () {
        $(document)
          .bind('click', function (e) {
            var $target = $(e.target);

            $(document)
              .find('.date-picker.show')
              .removeClass('show')
              .addClass('hide');

            $target
              .parents('.selecting-date')
              .find('.date-picker')
              .removeClass('hide')
              .addClass('show');
          });
      });

      $('form#inform-pupils .selecting-date.day').once('ditoolsiTrainingDayPicker', function () {
        var $container = $(this);

        $container
          .find('.date-picker')
          .addClass('hide');

        $container
          .find('.form-type-textfield')
          .find('.form-text')
          .bind('click', function () {
            $container
              .find('.date-picker')
              .removeClass('hide')
              .addClass('show');
          });

        $container
          .find('.form-type-textfield')
          .find('.form-text')
          .Zebra_DatePicker({
            select_other_months: false,
            always_visible: $container.find('.date-picker'),
            header_navigation: ['<span class="prev"></span>', '<span class="next"></span>'],
            // show_clear_date: false,
            months: [
              Drupal.t('January', {}, {context: 'calendar'}),
              Drupal.t('February', {}, {context: 'calendar'}),
              Drupal.t('March', {}, {context: 'calendar'}),
              Drupal.t('April', {}, {context: 'calendar'}),
              Drupal.t('May', {}, {context: 'calendar'}),
              Drupal.t('July', {}, {context: 'calendar'}),
              Drupal.t('June', {}, {context: 'calendar'}),
              Drupal.t('August', {}, {context: 'calendar'}),
              Drupal.t('September', {}, {context: 'calendar'}),
              Drupal.t('October', {}, {context: 'calendar'}),
              Drupal.t('November', {}, {context: 'calendar'}),
              Drupal.t('December', {}, {context: 'calendar'}),
            ],
            header_captions: {
              days: 'F Y',
              months: 'Y',
              years: 'Y1 - Y2'
            },
            onSelect: function(dateFormatted, dateOriginal, dateObj, element, row) {
              var day = dateObj.getDate(),
                  month = (dateObj.getMonth() + 1),
                  year = dateObj.getFullYear();

              dayNumber = day + ':' + month + ':' + year;

              var monthName = Drupal.settings.ditoolsiPlanning.months.cases[ month ];

              $container
                .find('.selected')
                .html(day + ' ' + monthName);

              $container
                .find('.form-item-date')
                .find('.form-text')
                .val(dayNumber)
                .attr('value', dayNumber);

              $container
                .find('.day-picker')
                .removeClass('show')
                .addClass('hide');

              var date = (year + '-' + month + '-' + day),
                  type = setting.ditoolsiPlanning.type;

              $.ajax({
                url: setting.ditoolsiPlanning.goalsUrl + '/' + date + '/' + type,
                dataType: 'json',
                success: function(response) {
                  if (response.html) {
                    $('#goals-lists').html(response.html);
                    Drupal.attachBehaviors();
                  }
                  else if (response.error) {
                    alertify.error(response.error);
                  }
                },
                error: function(response) {
                  if (response.status === 403) {
                    alertify.error(Drupal.t('Access denied'));
                  }
                  else {
                    alertify.error(Drupal.t('An error occurred. Please reload page and try newly'));
                  }
                }
              });
            }
          });

          var dateObj = new Date($container.find('.form-item-date-search-date-from').find('.form-text').val()),
              dayNumber = '';

          // $container
          //   .find('.selected')
          //   .html(dateObj.getDate() + ' ' + Drupal.settings.ditoolsiPlanning.months.cases[ dateObj.getMonth() + 1 ]);

          $container
            .find('.form-type-textfield')
            .find('.form-text')
            .val(dayNumber)
            .attr('value', dayNumber);
      });
    }
  };

  Drupal.behaviors.ditoolsiTrainingCheckbox = {
    attach: function() {
      $('.table-select-processed .form-type-checkbox>input[type="checkbox"]').each(function () {
        if ($(this).is(':checked')) {
          $(this).parents('tr').addClass('selected');
        }
      });
    }
  };

  Drupal.behaviors.ditoolsiTrainingFormLabel = {
    attach: function() {
      $('#inform-pupils')
        .find('.select-all .form-checkbox')
        .once('ditoolsiTrainingFormLabel', function () {
          var id = Math.random().toString().split('.')[1];

          $(this).attr('id', id);

          $('<label />', {
            for: id
          }).insertAfter($(this));
        });
    }
  };

})(jQuery);
