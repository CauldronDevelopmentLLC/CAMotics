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

    // Open external links in new tab
    $('.docs-section a, .bs-footer a').attr('target', '_blank');

    // Enable shadow box
    Shadowbox.init({displayNav: true});
});
