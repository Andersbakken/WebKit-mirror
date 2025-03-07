function stringifyObj(o)
{
    s = "";
    if (o)
        for (index in o) {
            s += index + ": " + o[index] + ";";
        }
    return s;
}

function onError(e)
{
    testFailed("Got error: " + stringifyObj(e));
    cleanUp();
}

function assert(s)
{
    if (!s)
        onError(new Error("Assertion failed. "));
}

var fileEntryForCleanup;  // Set as soon as we have one.

function cleanUp()
{
    try {
        if (fileEntryForCleanup)
            fileEntryForCleanup.remove(finishJSTest, finishJSTest);
    } catch (ex) {
        finishJSTest();
    }
}

// Generic function that gets a File from a FileEntry and calls a custom verification function on it.
function verifyFileContents(fileEntry, verifyFunc, arg0, arg1, onSuccess)
{
    fileEntry.file(
        function(file) {
            verifyFunc(file, arg0, arg1, onSuccess);
        },
        onError);
}

// Helper function used with verifyFileContents.
function verifyFileLengthHelper(file, length, unused, onSuccess)
{
    assert(file.size == length);
    onSuccess();
}

// Verifies that the contents of fileEntry have the supplied length.
function verifyFileLength(fileEntry, length, onSuccess)
{
    verifyFileContents(fileEntry, verifyFileLengthHelper, length, null, onSuccess);
}

// Helper function used with verifyFileContents.
function verifyByteRangeIsZeroesHelper(file, start, length, onSuccess)
{
    var fileReader = new FileReader();
    fileReader.onerror = onError;
    fileReader.onload =
        function() {
            var result = fileReader.result;
            for (var i = 0; i < length; i++)
                assert(result.charCodeAt(i) == 0);
            onSuccess();
        };
    fileReader.readAsBinaryString(file.slice(start, length));
}

// Verifies that fileEntry, at offset, is all zeroes for length bytes.
function verifyByteRangeIsZeroes(fileEntry, offset, length, onSuccess)
{
    verifyFileContents(fileEntry, verifyByteRangeIsZeroesHelper, offset, length, onSuccess);
}

// Helper function used with verifyFileContents.
function verifyByteRangeAsStringHelper(file, start, data, onSuccess)
{
    var fileReader = new FileReader();
    fileReader.onerror = onError;
    fileReader.onload =
        function() {
            assert(fileReader.result == data);
            onSuccess();
        };
    fileReader.readAsText(file.slice(start, data.length));
}

// Verifies that the contents of fileEntry, at offset, match contents [a string].
function verifyByteRangeAsString(fileEntry, offset, contents, onSuccess)
{
    verifyFileContents(fileEntry, verifyByteRangeAsStringHelper, offset, contents, onSuccess);
}

// Creates a file called fileName in fileSystem's root directory, truncates it to zero length just in case, and calls onSuccess, passing it a FileEntry and FileWriter for the new file.
function createEmptyFile(fileSystem, fileName, onSuccess)
{
    function getSuccessFunc(fileEntry, fileWriter) {
        return function() {
            onSuccess(fileEntry, fileWriter);
        }
    }
    function getFileWriterCallback(fileEntry) {
        return function(fileWriter) {
            var successFunc = getSuccessFunc(fileEntry, fileWriter);
            fileWriter.onError = onError;
            fileWriter.onwrite = function() {
                verifyFileLength(fileEntry, 0, successFunc);
            };
            fileWriter.truncate(0);
        }
    }
    function onFileEntry(fileEntry) {
        fileEntryForCleanup = fileEntry;
        var onFileWriter = getFileWriterCallback(fileEntry);
        fileEntry.createWriter(onFileWriter, onError);
    }
    assert(fileSystem);
    fileSystem.root.getFile(fileName, {create:true}, onFileEntry, onError);
}

function writeString(fileEntry, fileWriter, offset, data, onSuccess)
{
    var bb = new BlobBuilder();
    bb.append(data);
    var blob = bb.getBlob();
    fileWriter.seek(offset);
    fileWriter.write(blob);
    fileWriter.onwrite = function() {
        verifyByteRangeAsString(fileEntry, offset, data, onSuccess);
    };
}

function setupAndRunTest(size, testName, testFunc)
{
    if (!requestFileSystem) {
        debug("This test requires FileSystem API support.");
        return;
    }
    debug("starting test");
    requestFileSystem(TEMPORARY, size, function(fs) {
        createEmptyFile(fs, testName, testFunc);
    }, onError);
}

