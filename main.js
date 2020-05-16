function runCode(code, onstdout, onstderr) {
    MyCode({
        'print': onstdout,
        'printErr': onstderr,
    }).then(function (Module) {
        Module.FS.writeFile('/input.lang', code);
        Module.callMain(['/input.lang']);
    });
}

function appendStdout(text) {
    var div = $("<div>", {"class": "stdout-msg"});
    div.html(text);
    $('#result').append(div);
}

function appendStderr(text) {
    var div = $("<div>", {"class": "stderr-msg"});
    div.html(text);
    $('#result').append(div);
}

function clearResults() {
    $('#result').empty();
}

$('#vim-on').hide();

$(document).ready(function () {
    var elem = document.getElementById('cm');
    var cm = CodeMirror(elem, {
        value: '; Hello, world!\n' +
               '(print "Hello, world!")\n\n' +
               '; Simple GCD function...\n' +
               '(def gcd a b (? (= a 0) b (gcd (% b a) a)))\n' +
               '(print (gcd 60 35))\n\n' +
               '; Fast naive fibonacci (out-of-the-box lazy eval)\n' +
               '(def fib n (do\n' +
               '    (let a (fib (- n 1)))\n' +
               '    (let b (fib (- n 2)))\n' +
               '    (match n\n' +
               '        0   : 0\n' +
               '        1   : 1\n' +
               '        ANY : (+ a b)\n' +
               '    )\n' +
               '))\n' +
               '(print (fib 15))',
        mode: 'text/x-common-lisp',
        theme: 'monokai',
        lineNumbers: true,
        viewportMargin: Infinity,
        keyMap: 'sublime',
        extraKeys: {
            'Ctrl-Enter': function () {
                var code = cm.getValue();
                clearResults();
                runCode(code, appendStdout, appendStderr);
            },
            'Cmd-Enter': function () {
                var code = cm.getValue();
                clearResults();
                runCode(code, appendStdout, appendStderr);
            }
        }
    });

    $('#run').on('click', function () {
        var code = cm.getValue();
        clearResults();
        runCode(code, appendStdout, appendStderr);
    });

    $('#vim-off').on('click', function () {
        $('#vim-off').hide();
        $('#vim-on').show();
        cm.setOption('keyMap', 'vim');
    });
    $('#vim-on').on('click', function () {
        $('#vim-on').hide();
        $('#vim-off').show();
        cm.setOption('keyMap', 'sublime');
    });
}); 
