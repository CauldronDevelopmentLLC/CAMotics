function create_sidebar() {
    var top = $('#sidebar ul');
    var lastH1;
    var lastH2;

    $('#content h1, #content h2, #content .func').each(function () {
        var elem = $(this);
        var tag = elem.prop('tagName');
        var id = elem.attr('id');

        switch (tag) {
        case 'H1':
            lastH1 = $('<li>')
                .append($('<a>').attr('href', '#' + id)
                        .text(elem.text()))
                .appendTo(top);
            break;

        case 'H2':
            var ul = lastH1.find('> ul');
            if (!ul.length) ul = $('<ul>').appendTo(lastH1);

            lastH2 = $('<li>')
                .append($('<a>').attr('href', '#' + id)
                        .text(elem.text()))
                .appendTo(ul);
            break;

        case 'DIV':
            var ul = lastH2.find('> ul');
            if (!ul.length) ul = $('<ul>').appendTo(lastH2);

            var title = id.replace(/-/g, '.');

            $('<li>')
                .append($('<a>').attr('href', '#' + id)
                        .text(title + '()'))
                .appendTo(ul);
            break;
        }
    });
}


$(function() {
    // Open links in new window
    $('a').attr('target', '_blank');

    // Create sidebar menu
    create_sidebar();

    // Setup simulation images
    $('.sim-image')
        .css({width: '300px'})
        .click(function () {
            var w = $(this).css('width');
            $(this).css({width: w == '300px' ? 'auto' : '300px'});
        });

    // Syntax highlight code
    $('#content .example, #content .decl').attr('data-language', 'javascript');
    Rainbow.color();
});
