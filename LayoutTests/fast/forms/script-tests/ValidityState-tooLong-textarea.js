description('Tests for tooLong flag with &lt;textarea> elements.');

var textarea = document.createElement('textarea');
document.body.appendChild(textarea);

debug('No maxlength and no value');
shouldBeFalse('textarea.validity.tooLong');

debug('');
debug('Non-dirty value');
textarea.defaultValue = 'abcde';
textarea.maxLength = 3;
shouldBe('textarea.value.length', '5');
shouldBeFalse('textarea.validity.tooLong');

textarea.defaultValue = 'abcdef';
shouldBe('textarea.value.length', '6');
shouldBeFalse('textarea.validity.tooLong');

debug('');
debug('Dirty value and longer than maxLength');
textarea = document.createElement('textarea');
document.body.appendChild(textarea);
textarea.defaultValue = 'abcde';
textarea.maxLength = 3;
textarea.focus();
textarea.setSelectionRange(5, 5);  // Move the cursor at the end.
document.execCommand('delete');
shouldBe('textarea.value.length', '4');
shouldBeTrue('textarea.validity.tooLong');
// Make the value <=maxLength.
document.execCommand('delete');
shouldBeFalse('textarea.validity.tooLong');

debug('');
debug('Sets a value via DOM property');
textarea = document.createElement('textarea');
document.body.appendChild(textarea);
textarea.maxLength = 3;
textarea.value = 'abcde';
shouldBeTrue('textarea.validity.tooLong');

debug('');
debug('Disabled');
textarea.disabled = true;
shouldBeFalse('textarea.validity.tooLong');
textarea.disabled = false;

debug('');
debug('Grapheme length is not greater than maxLength though character length is greater');
// fancyX should be treated as 1 grapheme.
// U+0305 COMBINING OVERLINE
// U+0332 COMBINING LOW LINE
var fancyX = "x\u0305\u0332";
textarea = document.createElement('textarea');
document.body.appendChild(textarea);
textarea.value = fancyX; // 3 characters, 1 grapheme cluster.
textarea.maxLength = 1;
shouldBeFalse('textarea.validity.tooLong');

debug('');
debug('A value set by resetting a form or by a child node change doesn\'t make tooLong true.');
// Make a dirty textarea.
var parent = document.createElement('div');
document.body.appendChild(parent);
parent.innerHTML = '<form><textarea maxlength=2>abc</textarea></form>';
textarea = parent.firstChild.firstChild;
textarea.value = 'def';
shouldBeTrue('textarea.validity.tooLong');
parent.firstChild.reset();
shouldBe('textarea.value', '"abc"');
shouldBeFalse('textarea.validity.tooLong');

parent.innerHTML = '<textarea maxlength=2>abc</textarea>';
textarea = parent.firstChild;
textarea.value = 'def';
shouldBeTrue('textarea.validity.tooLong');
parent.firstChild.innerHTML = 'abcdef';
shouldBe('textarea.value', '"abcdef"');
shouldBeFalse('textarea.validity.tooLong');

var successfullyParsed = true;
