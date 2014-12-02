#include <include/v8.h>
#include <include/libplatform/libplatform.h>
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

// Extracts a C string from a V8 Utf8Value.
const char* ToCString(const v8::String::Utf8Value& value);

class ShellArrayBufferAllocator : public v8::ArrayBuffer::Allocator {
 public:
  virtual void* Allocate(size_t length);
  virtual void* AllocateUninitialized(size_t length);
  virtual void Free(void* data, size_t);
};



namespace V8Lib	{

class V8Wrapper
{
		ShellArrayBufferAllocator array_buffer_allocator;	
		v8::Platform* platform;
		v8::Isolate* isolate;


public:
		V8Wrapper();
		~V8Wrapper();
		bool RunFile(const char * name);

		v8::Handle<v8::Context> CreateShellContext(v8::Isolate* isolate);
		
};


void loadFile(const v8::FunctionCallbackInfo<v8::Value>& args);
void newCharacteristic(const v8::FunctionCallbackInfo<v8::Value>& args)	;
void newBuffer(const v8::FunctionCallbackInfo<v8::Value>& args);
void newAnomaly(const v8::FunctionCallbackInfo<v8::Value>& args);


v8::Handle<v8::String> ReadFile(v8::Isolate* isolate, const char* name);
// Executes a string within the current v8 context.
bool ExecuteString(v8::Isolate* isolate,
                   v8::Handle<v8::String> source,
                   v8::Handle<v8::Value> name,
                   bool print_result,
                   bool report_exceptions);
void ReportException(v8::Isolate* isolate, v8::TryCatch* try_catch);	

}

