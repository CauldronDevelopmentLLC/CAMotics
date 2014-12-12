$(function() {
    // IE10 viewport hack for Surface/desktop Windows 8 bug
    if (navigator.userAgent.match(/IEMobile\/10\.0/)) {
        var msViewportStyle = document.createElement('style')
        msViewportStyle.appendChild(
            document.createTextNode('@-ms-viewport{width:auto!important}'));

        document.querySelector('head').appendChild(msViewportStyle);
    }

    var $window = $(window);
    var $body = $(document.body);

    $body.scrollspy({target: '.sidebar'});

    $window.on('load', function() {$body.scrollspy('refresh');});
    $('.docs-container [href=#]').click(function(e) {e.preventDefault();});

    // back to top
    setTimeout(function () {
        var $sideBar = $('.sidebar');

        $sideBar.affix({
            offset: {
                top: function() {
                    var offsetTop = $sideBar.offset().top;
                    var sideBarMargin =
                        parseInt($sideBar.children(0).css('margin-top'), 10);
                    var navOuterHeight = $('.docs-nav').height();

                    this.top = offsetTop - navOuterHeight - sideBarMargin;
                    return this.top;
                },

                bottom: function() {
                    this.bottom = $('.bs-footer').outerHeight(true);
                    return this.bottom;
                }
            }
        })
    }, 100);

    setTimeout(function() {$('.top').affix();}, 100);

    // Make releases clickable
    $('.release').click(function (e) {
        var origin = window.location;
        var target = $(this).find('a').attr('href');
        window.location = target;

        $.get('http://c.statcounter.com/click.gif', {
            sc_project: sc_project,
            security: sc_security,
            c: '' + target,
            m: 2,
            u: '' + origin,
            t: document.title,
            rand: Math.random()
        });

        e.preventDefault();
    });

    // Open external links in new tab
    $('a').each(function (index, a) {
        a = $(a);
        if (a.attr('href').indexOf('//') != -1)
            a.attr('target', '_blank');
    });

    // Enable shadow box
    Shadowbox.init({displayNav: true});

    // Load GitHub iframes
    var base = 'http://ghbtns.com/github-btn.html?' +
        'user=CauldronDevelopmentLLC&repo=OpenSCAM&count=true'
    $('<iframe>')
        .attr({src: base + '&type=watch', title: 'Star on GitHub',
               width: 100, height: 20})
        .addClass('github-btn')
        .appendTo('.github-star');
    $('<iframe>')
        .attr({src: base + '&type=fork', title: 'Fork on GitHub',
               width: 100, height: 20})
        .addClass('github-btn')
        .appendTo('.github-fork');

    // Banner
    var banner = $('.banner');
    var bannerID = banner.attr('id');

    if (document.cookie.indexOf('banner-close=' + bannerID) == -1)
        banner.slideDown('fast');

    $('.banner-close').click(function () {
        banner.slideUp('fast');
        document.cookie = 'banner-close=' + bannerID + '; path=/';
    });

    // Google Analytics
    GoogleAnalyticsObject = 'ga';
    ga = {
        'q': [['create', 'UA-57023811-1', 'auto'], ['send', 'pageview']],
        'l': 1 * new Date()
    };
    $('<script>')
        .attr({'src': '//www.google-analytics.com/analytics.js', 'async': 1})
        .appendTo('head');
});
