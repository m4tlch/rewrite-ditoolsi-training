function getCookie(name) {
    var matches = document.cookie.match(new RegExp(
        "(?:^|; )" + name.replace(/([\.$?*|{}\(\)\[\]\\\/\+^])/g, '\\$1') + "=([^;]*)"
    ));
    return matches ? decodeURIComponent(matches[1]) : undefined;
}

var player;

function onYouTubeIframeAPIReady() {
    var vid = document.getElementById('vid_id');

    if (getCookie('video') != 'watched' && vid && vid.innerText) {
        jQuery('#myModal').modal('show');
        var id = vid.innerText;
        vid.remove();
        player = new YT.Player('video', {
            height: '420',
            width: '640',
            videoId: id,
            events: {
                'onReady': onPlayerReady,
                'onStateChange': onPlayerStateChange
            }
        });
    }
}

function onPlayerReady(event) {
    event.target.playVideo();
    document.cookie = "video=watched";
}

function onPlayerStateChange(event) {
    if (event.data === 0) {
        jQuery('#myModal').modal('hide');
    }
}

(function ($) {

    "use strict";

    setInterval(function () {
        if ($('#vid_id').length && YT && YT.Player) {
            onYouTubeIframeAPIReady();
        }
    }, 1000);

    var matches = [];

    $(document).ready(function () {
        window.autoTestInterval = [];
        $('.lesson-embedded-video iframe').css({
            height: ($('.lesson-embedded-video iframe').width() * (9 / 16))

        });

    });

    Drupal.behaviors.ditoolsiShowVideo = {
        attach: function () {

            $('#myModal').on('hidden.bs.modal', function (e) {
                player.stopVideo();
            });
        }
    };

    Drupal.behaviors.ditoolsiTrainingLessonTabs = {
        attach: function () {
            setTimeout(function () {
                $('#to-video, #to-audio, #to-presentation, #to-task, #lesson-tabs').each(function () {
                    if ($(this).data('sliderTabs')) {
                        $(this).data('sliderTabs').resizeContentContainer();
                    }
                });
            }, 600);

            $('#lesson-tabs').once('ditoolsiTrainingVideoTabs', function () {
                var sliderTabsConfig = {
                    panelArrows: false,
                    mousewheel: false,
                    transitionEasing: 'easeInOutCirc'
                };

                matches = location.hash.match(/^#to-task-(\d+)/);

                if (matches && !isNaN(parseInt(matches[1], 10))) {
                    if ($('#lesson-tabs > ul li a[href="#to-task"]').length) {
                        sliderTabsConfig.defaultTab = $('#lesson-tabs > ul li a[href="#to-task"]').parent().index() + 1;
                    }
                }

                $(this).sliderTabs(sliderTabsConfig);

                var $nav = $('#lesson-tabs'),
                    $border = $('<div />', {
                        id: 'lesson-tabs-border'
                    }).appendTo($nav);

                function rebuildBorder() {
                    var style = '',
                        defaultLeft = Math.round($nav.find('li.selected').offset().left - $nav.offset().left),
                        defaultTop = $nav.find('li.selected').height(),
                        defaultWidth = $nav.find('li.selected').outerWidth();

                    $border.css({
                        left: defaultLeft,
                        top: defaultTop,
                        width: defaultWidth
                    });
                }

                $nav.bind('changedTab', rebuildBorder);

                rebuildBorder();
            });

            var ids = [];

            if ($('#to-video').length) {
                ids.push('#to-video');
            }
            if ($('#to-audio').length) {
                ids.push('#to-audio');
            }
            if ($('#to-presentation').length) {
                ids.push('#to-presentation');
            }

            if (ids.length) {
                $(ids.join(',')).once('ditoolsiTrainingVideoTabs', function () {
                    var sliderTabsConfig = {
                        panelArrows: false,
                        mousewheel: false,
                        transitionEasing: 'easeInOutCirc'
                    };

                    $(this).sliderTabs(sliderTabsConfig);
                    $(this).bind('changedTab', function () {
                        var $tabs = $(this);

                        setTimeout(function () {
                            $('#lesson-tabs').data('sliderTabs').resizeContentContainer();
                        }, 500);
                    });
                });
            }
            if ($('body').hasClass('uid-32405')) {

            }
            else {
                if ($('#to-task').length) {
                    var sliderTabsConfig = {
                        panelArrows: false,
                        mousewheel: false,
                        transitionEasing: 'easeInOutCirc'
                    };

                    matches = location.hash.match(/^#to-task-(\d+)/);

                    if (matches && !isNaN(parseInt(matches[1], 10))) {
                        if ($('#task-switcher-tabs li a[href="#to-task-' + matches[1] + '"]').length) {
                            sliderTabsConfig.defaultTab = parseInt(matches[1], 10) + 1;
                        }
                    }

                    $('#to-task').sliderTabs(sliderTabsConfig);
                }
            }

        }
    };

    Drupal.behaviors.ditoolsiTrainingLessonVideo = {
        attach: function () {

            $('#to-video, #to-presentation')
                .find('video')
                .once('ditoolsiTrainingLessonVideo', function () {
                    var $object = $(this);

                    $('#lesson-tabs')
                        .bind('changedTab', function () {
                            $object.css({
                                height: ($object.width() * (9 / 16))
                            });
                        })
                        .find('.ui-slider-tab-content')
                        .bind('changedTab', function () {
                            $object.css({
                                height: ($object.width() * (9 / 16))
                            });
                        });

                    $object.css({
                        height: ($object.width() * (9 / 16))

                    });

                });
        }
    };

    Drupal.behaviors.ditoolsiTrainingNextVideo = {
        attach: function () {
            $('#to-video')
                .find('.next-video')
                .once('ditoolsiTrainingNextVideo', function () {
                    $(this).bind('click', function () {
                        var k = parseInt($(this).data('k')),
                            n = 0;

                        $('#to-video')
                            .data('sliderTabs')
                            .selectTab(n + 2);
                    });
                });
        }
    };

    Drupal.behaviors.ditoolsiTrainingNextPresentation = {
        attach: function () {
            $('#to-presentation')
                .find('.next-presentation')
                .once('ditoolsiTrainingNextPresentation', function () {
                    $(this).bind('click', function () {
                        var k = parseInt($(this).data('k')),
                            n = 0;

                        $('#to-presentation')
                            .data('sliderTabs')
                            .selectTab(n + 2);
                    });
                });
        }
    };

    Drupal.behaviors.ditoolsiTrainingAutotest = {
        attach: function (context, setting) {
            setTimeout(function () {
                if (Drupal.settings.dt && Drupal.settings.dt.startAutotest) {
                    var taskId, $timer,
                        startedTime, time,
                        diff, hours,
                        minutes, seconds,
                        htmlTime = '', settings = {},
                        ajax;

                    $.each(Drupal.settings.dt.startAutotest, function (index, value) {
                        taskId = value.taskId;
                        time = value.time;
                        startedTime = value.startedTime;

                        if (Drupal.settings.dt.recast && Drupal.settings.dt.recast[taskId] !== false) {
                            startedTime = Drupal.settings.dt.recast[taskId];
                            console.log(startedTime);
                            Drupal.settings.dt.startAutotest[index].startedTime = startedTime;
                            Drupal.settings.dt.recast[taskId] = false;
                            if (autoTestInterval[taskId]) {
                                clearInterval(autoTestInterval[taskId]);
                                autoTestInterval[taskId] = false;
                            }
                        }

                        settings.url = value.checkPath;
                        ajax = new Drupal.ajax(false, false, settings);
                        ajax.error = function () {
                        };

                        if (autoTestInterval[taskId]) {
                            return;
                        }

                        $('#autotest-' + taskId)
                            .find('.autotest-time')
                            .remove();
                        $('#autotest-' + taskId).prepend($timer);

                        $timer = $('<div />', {
                            class: 'autotest-timer'
                        }).prependTo('#autotest-' + taskId);

                        autoTestInterval[taskId] = setInterval(function () {
                            diff = (startedTime + time) - Math.round(new Date().getTime() / 1000);
                            ajax.eventResponse(ajax, {});

                            if (diff < 0) {
                                clearInterval(autoTestInterval[taskId]);
                                return;
                            }

                            hours = Math.floor(diff / 3600);
                            minutes = Math.floor((diff - (hours * 3600)) / 60);
                            seconds = diff - (hours * 3600) - (minutes * 60);

                            if (hours < 10) hours = '0' + hours;
                            if (minutes < 10) minutes = '0' + minutes;
                            if (seconds < 10) seconds = '0' + seconds;

                            htmlTime = '';
                            htmlTime += hours + ':';
                            htmlTime += minutes + ':';
                            htmlTime += seconds;

                            $timer.html(htmlTime);
                        }, 1000);
                    });
                }
            }, 400);
        }
    };

    Drupal.behaviors.ditoolsiTrainingLessonSelectFile = {
        attach: function () {
            var $label, $selected;

            function updateFileValue(selector) {
                if (selector.files.length) {
                    $('div.selected-file[for="' + selector.id + '"]').html(selector.files[0].name);
                }
                else {
                    $('div.selected-file[for="' + selector.id + '"]').html(Drupal.t('No file chosen'));
                }
            }

            $('form[id^="save-task-form-"] .field-name-field-progress-files,\
        form[id^="edit-texttask-form"] .form-item-text-task-file')
                .find('.form-file')
                .once('ditoolsiTrainingLessonSelectFile', function () {
                    $label = $('<label />', {
                        for: $(this).attr('id'),
                        class: 'label-add-file'
                    }).html(Drupal.t('Select a file'));
                    $selected = $('<div />', {
                        class: 'selected-file',
                        for: $(this).attr('id')
                    });

                    $(this).before($label).before($selected).hide();
                    $(this).bind('change', function (e) {
                        updateFileValue(this);
                    });

                    updateFileValue(this);
                });
        }
    };

    Drupal.behaviors.ditoolsiTrainingToggleForm = {
        attach: function () {
            $('a.toggle-form').once('ditoolsiTrainingToggleForm', function () {
                $(this).bind('click', function (e) {
                    e.preventDefault();

                    if ($(this).hasClass('to-fix')) {
                        $('.accept-task-form').hide();
                        $('.to-fix-task-form').show();
                    }
                    else if ($(this).hasClass('accept')) {
                        $('.accept-task-form').show();
                        $('.to-fix-task-form').hide();
                    }
                });
            });
        }
    };

    Drupal.behaviors.dtVideoSize = {
        attach: function () {
            $('video').once('dtVideoSize', function () {
                var $video = $(this);
                setInterval(function () {
                    $video.css({
                        height: $video.width() / 16 * 9
                    });
                }, 1000);
            });
        }
    };

    Drupal.behaviors.dtSaveLessonPopup = {
        attach: function () {
            $('#task-saved-popup').once('dtSaveLessonPopup', function () {
                $('html, body').animate({
                    scrollTop: 0
                });
            });
            $('#task-saved-popup a').once('dtSaveLessonPopup', function () {
                $(this).on('click', function (e) {
                    if ($(this).data('idd')) {
                        e.preventDefault();
                        var id = $('#to-task #task-switcher-tabs li.selected a').attr('href').replace('#to-task-', '');
                        id = parseInt(id) + 1;
                        $('#task-switcher-tabs li a[href="#to-task-' + id + '"]').click();
                        $('#task-saved-popup, #ditoolsi-overlay').remove();
                        return false;
                    }
                });
            });
        }
    };
    Drupal.behaviors.mgcSaveLessonPopup = {
        attach: function () {
            var $text = '<span class="text-desc">Вы выполнили все задания к уроку, отправьте урок на проверку</span>';
            var $send_button = $('.send-to-review');
            if ($send_button.length > 0) {
                $('body').once('mgcSaveLessonPopup', function () {
                    $(this).append('<div id="task-saved-popup">' + $text + '</div><div id="ditoolsi-overlay"></div>');
                    $send_button.appendTo('.text-desc');
                    $('html, body').animate({
                        scrollTop: 0
                    });
                });
            }
        }
    };

    Drupal.behaviors.dtLinkToTask = {
        attach: function () {
            $('#drupal-messages a').once('dtLinkToTask', function () {
                if (this.hash.match(/^#to-task-(\d+)/)) {
                    $(this).on('click', function () {
                        var matches = this.hash.match(/^#to-task-(\d+)/);

                        if (matches && !isNaN(parseInt(matches[1], 10))) {
                            $('#lesson-tabs li a[href="#to-task"]').click();
                            if ($('#task-switcher-tabs li a[href="' + this.hash + '"]').length) {
                                var hash = this.hash;
                                setTimeout(function () {
                                    $('#task-switcher-tabs li a[href="' + hash + '"]').click();
                                    $('html, body').animate({
                                        scrollTop: $('#to-task').offset().top - 20
                                    });
                                }, 800);
                                // $('html, body').animate({
                                //   scrollTop: $('#to-task').offset().top - 20
                                // });
                            }
                            // $('#lesson-tabs > ul li a[href="#to-task"]')
                        }
                    });
                }
            });
        }
    };

    Drupal.ajax.prototype.commands.ditoolsiTrainingSwitchTask = function (ajax, response, status) {
        var task = parseInt(response.task) - 1;

        $('#to-task #task-switcher-tabs li a[href="#to-task-' + (task - 1) + '"').attr('class', response.class);

        if (!$('#to-task #task-switcher-tabs li a[href="#to-task-' + task + '"').length) {
            window.location.reload();
        } else {
            $('#to-task #task-switcher-tabs li a[href="#to-task-' + task + '"').click();
        }
    };

})(jQuery);
