Javascript Example
==================

This is just a simple example of embedding the Google V8 engine within C++ code.

Example usage:
'''
V8Lib::V8Wrapper v8wrap;
v8wrap.RunFile("/buildv8/sample.js");
'''

Sample Javascript:
'''javascript

loadLibrary("/buildv8/library.js");

function def() 	{
	runBuffer("dfdfdf","asdasd");
}

abc();


newBuffer("asdsadasdasd");
'''