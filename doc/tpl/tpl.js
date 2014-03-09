$(function() {
    var sidebar = $('#sidebar');
    var content = $('#content');

    // Create sidebar menu & modify sections
    $.each(content.find('div[id]'), function(i, div) {
        div = $(div)
        var id = div.attr('id');

        var title = div.attr('title');
        if (typeof title == 'undefined') title = id;
        else if (div.hasClass('L0') || div.hasClass('L1'))
            div.prepend($('<h2>').addClass('title').text(title));
        else if (div.hasClass('L2'))
            div.prepend($('<h3>').addClass('title').text(title));

        div.addClass('level');

        var entry = $('<div><a href="#' + id + '">' + title + '</a></div>');
        entry.addClass(div.attr('class'));
        entry.addClass('level');

        sidebar.append(entry);
    });


    // Fix example indentation
    $.each(content.find('.example'), function(i, example) {
        example = $(example);

        // Split lines
        var lines = example.html().match(/([^\r\n]*)\r?\n\r?/g);
        if (lines == null) return;

        // Compute indent
        var indent = 0;
        for (var i = 0; i < lines.length; i++) {
            lines[i] = lines[i].replace(/\s+$/g, '');
            if (!lines[i].length) continue;

            var len = lines[i].match(/^\s*/)[0].length;
            if (!indent || len < indent) indent = len;
        }

        // Remove indent
        if (indent)
            for (var i = 0; i < lines.length; i++)
                if (indent < lines[i].length)
                    lines[i] = lines[i].substr(indent);

        // Rejoin lines
        var html = '';
        for (var i = 0; i < lines.length; i++) {
            if (html) html += '\n';
            html += lines[i];
        }

        // Reset contents
        example.html(html);
    });


    // Setup simulation images
    $('.sim-image')
        .css({width: '300px'})
        .click(function () {
            var w = $(this).css('width');
            $(this).css({width: w == '300px' ? 'auto' : '300px'});
        });

    function colorize(i, div) {
        var div = $(div);
        var code = $('<code>')
            .attr({'data-language': 'javascript'})
            .append(div.html());
        div.html(code);
    }

    $.each(content.find('.example'), colorize);
    $.each(content.find('.func .decl'), colorize);
    Rainbow.color();
});
