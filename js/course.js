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

    Drupal.behaviors.ditoolsiTrainingSelectLesson = {
        attach: function () {

            $('.lessons-list .lesson-number')
                .once('ditoolsiTrainingSelectLesson', function () {
                    $(this).bind('click', function () {
                        var $item = $(this).parents('.lesson-list-item');

                        $('.lessons-list')
                            .find('.lesson-list-item')
                            .not($item)
                            .removeClass('selected');
                        var redirect_url = $item.find('.edit-link a').prop('href');
                        if ($('body').hasClass('куратор')) {
                            redirect_url = redirect_url.replace('/edit', '');
                        }
                        $(location).attr('href', redirect_url);

                        //$item.toggleClass('selected');
                    });
                });
        }
    };

    Drupal.behaviors.ditoolsiShowVideo = {
        attach: function () {

            $('#myModal').on('hidden.bs.modal', function (e) {
                player.stopVideo();
            });

        }
    };

    Drupal.behaviors.ditoolsiTrainingReadmore = {
        attach: function () {
            $('.course-body')
                .once('ditoolsiTrainingReadmore', function () {
                    $(this).readmore({
                        speed: 350,
                        lessLink: '<a class="readmore">' + Drupal.t('Hide') + ' <i class="fa fa-angle-up"></i></a>',
                        moreLink: '<a class="readmore">' + Drupal.t('Read more') + ' <i class="fa fa-angle-down"></i></a>'
                    });
                });
        }
    };

    Drupal.behaviors.ditoolsiTrainingBlockedPopup = {
        attach: function () {
            $('.block-user-popup .link-close a').once('ditoolsiTrainingBlockedPopup', function () {
                $(this).bind('click', function () {
                    $('.block-user-popup').remove();
                });
            });
        }
    };

    Drupal.behaviors.dtTrainingFirstSessionVideo = {
        attach: function (context, setting) {
            $('body').once('dtTrainingFirstSessionVideo', function () {
                if ($.cookie('dt_profile_start') && setting.dtTraining.firstSessionVideo) {
                    var url = setting.dtTraining.firstSessionVideo,
                        regExp = /^.*((youtu.be\/)|(v\/)|(\/u\/\w\/)|(embed\/)|(watch\?))\??v?=?([^#\&\?]*).*/,
                        match = url.match(regExp),
                        mrgLeft = $(document).outerWidth(true) / 4;

                    if (match && match[7].length == 11) {
                        var $overlay = $('<div />', {
                                id: 'ditoolsi-overlay'
                            }),
                            $videoContainer = $('<div />', {
                                id: 'training-video',
                                style: 'margin-left: -' + mrgLeft + 'px; height: ' + mrgLeft + 'px',
                            }),
                            $close = $('<a />', {
                                class: 'close-popup'
                            })
                                .html('<i class="fa fa-times"></i>').bind('click', function () {
                                    $('#ditoolsi-overlay, #training-video').remove();
                                })
                                .bind('click', function () {
                                    $.cookie('dt_profile_start', null, {
                                        path: '/'
                                    });
                                })
                                .appendTo($videoContainer),
                            $video = $('<iframe />', {
                                src: 'http://www.youtube.com/embed/' + match[7],
                                allowFullScreen: 'allowFullScreen'
                            });

                        $('body').append($videoContainer.append($video)).append($overlay);
                    }
                }
            });
        }
    };

    Drupal.behaviors.dtTrainingRequestCourse = {
        attach: function (context, setting) {
            $('.lessons-list .lesson-info span').once('dtTrainingRequestCourse', function () {
                $(this).on('click', function () {
                    if (!$(this).data('pass')) {
                        return;
                    }
                    if ($(this).data('access')) {
                        var courseId = $(this).data('courseId');
                        $('body').append('<div id="ditoolsi-overlay"></div>\
                       <div id="request-course"><a href="#" class="close"><i class="fa fa-times"></i></a> Для доступа к этому курсу, вам нужно <a href="' + setting.basePath + '?q=course/' + courseId + '/request">отправить заявку и купить курс</a>.</div>');
                    } else if (!$(this).data('started')) {
                        $('html, body').animate({
                            scrollTop: 0
                        });
                        $('body').append('<div id="ditoolsi-overlay"></div>\
                       <div id="request-course"><a href="#" class="close"><i class="fa fa-times"></i></a> Для доступа у уроку нажмите на кнопку "Начать обучение".</div>');
                    } else {
                        $('html, body').animate({
                            scrollTop: 0
                        });
                        $('body').append('<div id="ditoolsi-overlay"></div>\
                       <div id="request-course"><a href="#" class="close"><i class="fa fa-times"></i></a> Урок будет доступен после выполнения и проверки заданий предыдущего урока.</div>');
                    }

                    $('#request-course .close').on('click', function (e) {
                        e.preventDefault();
                        $('#ditoolsi-overlay, #request-course').remove();
                        return false;
                    });
                });
            });
        }
    };

})(jQuery);
