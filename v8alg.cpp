
#include "v8alg.h"

using namespace V8Lib;
#define DEBUG(x) std::cout << "DEBUG: " << x << std::endl;

// Extracts a C string from a V8 Utf8Value.
const char* ToCString(const v8::String::Utf8Value& value) {
  return *value ? *value : "<string conversion failed>";
}


void* ShellArrayBufferAllocator::Allocate(size_t length) {
    void* data = AllocateUninitialized(length);
    return data == NULL ? data : memset(data, 0, length);
}
void* ShellArrayBufferAllocator::AllocateUninitialized(size_t length) { return malloc(length); }
void ShellArrayBufferAllocator::Free(void* data, size_t) { free(data); }



V8Lib::V8Wrapper::V8Wrapper()
	: platform(0),isolate(0)
{
	v8::V8::InitializeICU();
	platform = v8::platform::CreateDefaultPlatform();
	v8::V8::InitializePlatform(platform);
	v8::V8::Initialize();
	isolate = v8::Isolate::New();
	v8::V8::SetArrayBufferAllocator(&array_buffer_allocator);

	
}


V8Lib::V8Wrapper::~V8Wrapper()	{
  v8::V8::Dispose();
  v8::V8::ShutdownPlatform();

  delete platform;
}

bool V8Lib::V8Wrapper::RunFile(const char* filename)	{


  {
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Handle<v8::Context> context = CreateShellContext(isolate);
    if (context.IsEmpty()) {
      fprintf(stderr, "Error creating context\n");
      return 1;
    }
    v8::Context::Scope context_scope(context);
	DEBUG("RunFile")
	v8::Handle<v8::String> source = ReadFile(isolate,filename);
	if (!ExecuteString(isolate,
                       source,
                       v8::String::NewFromUtf8(isolate, "poo"),
                       true,
                       true)) {
      isolate->ThrowException(
          v8::String::NewFromUtf8(isolate, "Error executing file"));
      return false;
    }

  }
  return true;

}

// Creates a new execution environment containing the built-in
// functions.
v8::Handle<v8::Context> V8Lib::V8Wrapper::CreateShellContext(v8::Isolate* isolate) {
	  // Create a template for the global object.
	  v8::Handle<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);
	  // Bind the global 'print' function to the C++ Print callback.
	  global->Set(v8::String::NewFromUtf8(isolate, "newBuffer"),
	              v8::FunctionTemplate::New(isolate, V8Lib::newBuffer));
	  // Bind the global 'read' function to the C++ Read callback.
	  global->Set(v8::String::NewFromUtf8(isolate, "newAnomaly"),
	              v8::FunctionTemplate::New(isolate, V8Lib::newAnomaly));
	  // Bind the global 'load' function to the C++ Load callback.
	  global->Set(v8::String::NewFromUtf8(isolate, "newCharacteristic"),
	              v8::FunctionTemplate::New(isolate, V8Lib::newCharacteristic));

	  global->Set(v8::String::NewFromUtf8(isolate, "loadLibrary"),
	              v8::FunctionTemplate::New(isolate, V8Lib::loadFile));

	  return v8::Context::New(isolate, NULL, global);
}

// Reads a file into a v8 string.
v8::Handle<v8::String> V8Lib::ReadFile(v8::Isolate* isolate, const char* name) {
	DEBUG("ReadFile");
	FILE* file = fopen(name, "rb");


	if (file == NULL) return v8::Handle<v8::String>();



	fseek(file, 0, SEEK_END);
	int size = ftell(file);
	rewind(file);

	char* chars = new char[size + 1];


	chars[size] = '\0';


	for (int i = 0; i < size;) {
		int read = static_cast<int>(fread(&chars[i], 1, size - i, file));
		i += read;
	}
	fclose(file);



	v8::Handle<v8::String> result =
	//  v8::String::NewFromUtf8(isolate, chars, v8::String::kNormalString, size);
	v8::String::NewFromUtf8(isolate, chars, v8::String::kNormalString, size);


	delete[] chars;
	DEBUG("end read file")
	return result;
}


void V8Lib::loadFile(const v8::FunctionCallbackInfo<v8::Value>& args){


  for (int i = 0; i < args.Length(); i++) {
    v8::HandleScope handle_scope(args.GetIsolate());
    v8::String::Utf8Value file(args[i]);
    if (*file == NULL) {
      args.GetIsolate()->ThrowException(
          v8::String::NewFromUtf8(args.GetIsolate(), "Error loading file"));
      return;
    }
    v8::Handle<v8::String> source = ReadFile(args.GetIsolate(), *file);
    if (source.IsEmpty()) {
      args.GetIsolate()->ThrowException(
           v8::String::NewFromUtf8(args.GetIsolate(), "Error loading file"));
      return;
    }
    if (!ExecuteString(args.GetIsolate(),
                       source,
                       v8::String::NewFromUtf8(args.GetIsolate(), *file),
                       false,
                       false)) {
      args.GetIsolate()->ThrowException(
          v8::String::NewFromUtf8(args.GetIsolate(), "Error executing file"));
      return;
    }
  }

}

void V8Lib::newCharacteristic(const v8::FunctionCallbackInfo<v8::Value>& args)	{

}

void V8Lib::newBuffer(const v8::FunctionCallbackInfo<v8::Value>& args)
{
  for (int i = 0; i < args.Length(); i++) {
    v8::HandleScope handle_scope(args.GetIsolate());

    v8::String::Utf8Value str(args[i]);
    const char* cstr = ToCString(str);
    std::cout << cstr << " ";
  }
  std::cout << std::endl;
  

}

void V8Lib::newAnomaly(const v8::FunctionCallbackInfo<v8::Value>& args){}

// Executes a string within the current v8 context.
bool V8Lib::ExecuteString(v8::Isolate* isolate,
                   v8::Handle<v8::String> source,
                   v8::Handle<v8::Value> name,
                   bool print_result,
                   bool report_exceptions) {
  v8::HandleScope handle_scope(isolate);
  v8::TryCatch try_catch;
  v8::ScriptOrigin origin(name);
  v8::Handle<v8::Script> script = v8::Script::Compile(source, &origin);
  if (script.IsEmpty()) {
    // Print errors that happened during compilation.
    if (report_exceptions)
      ReportException(isolate, &try_catch);
    return false;
  } else {
    v8::Handle<v8::Value> result = script->Run();
    if (result.IsEmpty()) {
      assert(try_catch.HasCaught());
      // Print errors that happened during execution.
      if (report_exceptions)
        ReportException(isolate, &try_catch);
      return false;
    } else {
      assert(!try_catch.HasCaught());
      if (print_result && !result->IsUndefined()) {
        // If all went well and the result wasn't undefined then print
        // the returned value.
        v8::String::Utf8Value str(result);
        const char* cstr = ToCString(str);
        printf("%s\n", cstr);
      }
      return true;
    }
  }
}






void V8Lib::ReportException(v8::Isolate* isolate, v8::TryCatch* try_catch) {
  v8::HandleScope handle_scope(isolate);
  v8::String::Utf8Value exception(try_catch->Exception());
  const char* exception_string = ToCString(exception);
  v8::Handle<v8::Message> message = try_catch->Message();
  if (message.IsEmpty()) {
    // V8 didn't provide any extra information about this error; just
    // print the exception.
    fprintf(stderr, "%s\n", exception_string);
  } else {
    // Print (filename):(line number): (message).
    v8::String::Utf8Value filename(message->GetScriptOrigin().ResourceName());
    const char* filename_string = ToCString(filename);
    int linenum = message->GetLineNumber();
    fprintf(stderr, "%s:%i: %s\n", filename_string, linenum, exception_string);
    // Print line of source code.
    v8::String::Utf8Value sourceline(message->GetSourceLine());
    const char* sourceline_string = ToCString(sourceline);
    fprintf(stderr, "%s\n", sourceline_string);
    // Print wavy underline (GetUnderline is deprecated).
    int start = message->GetStartColumn();
    for (int i = 0; i < start; i++) {
      fprintf(stderr, " ");
    }
    int end = message->GetEndColumn();
    for (int i = start; i < end; i++) {
      fprintf(stderr, "^");
    }
    fprintf(stderr, "\n");
    v8::String::Utf8Value stack_trace(try_catch->StackTrace());
    if (stack_trace.length() > 0) {
      const char* stack_trace_string = ToCString(stack_trace);
      fprintf(stderr, "%s\n", stack_trace_string);
    }
  }
}