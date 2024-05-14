#include <stdlib.h>
#include <array>
#include "tflm_esp32.h"
#include "utility.h"
#include "conv.h"

namespace {
  tflite::ErrorReporter* error_reporter = nullptr;
  const tflite::Model* model = nullptr;
  tflite::MicroInterpreter* interpreter = nullptr;
  TfLiteTensor* input = nullptr;
  TfLiteTensor* output = nullptr;
  // occupa tanto, meno di così non di può fare
  constexpr int kTensorArenaSize = 70 * 1024;
  uint8_t tensor_arena[kTensorArenaSize];
}

void setup_model(){
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;

  model = ::tflite::GetModel(conv_tflite);
  if(model->version() != TFLITE_SCHEMA_VERSION){
    TF_LITE_REPORT_ERROR(error_reporter,
    "Il modello non è di una versione supportata (model %d, supported %d)",
    model->version(), TFLITE_SCHEMA_VERSION);
  }

  //Aggiunta di tutte le operazioni richieste
  static tflite::MicroMutableOpResolver<11> resolver;
  resolver.AddCast();
  resolver.AddQuantize();
  resolver.AddSub();
  resolver.AddMul();
  resolver.AddExpandDims();
  resolver.AddConv2D();
  resolver.AddReshape();
  resolver.AddRelu();
  resolver.AddMaxPool2D();
  resolver.AddFullyConnected();
  resolver.AddDequantize();

  static tflite::MicroInterpreter static_interpreter(
    model,
    resolver,
    tensor_arena,
    kTensorArenaSize,
    error_reporter
  );
  interpreter = &static_interpreter;

  //andrebbe fatto il check di allocazione, ma non mi va
  TfLiteStatus allocate_status = interpreter->AllocateTensors();

  input = interpreter->input(0);
  output = interpreter->output(0);
}

std::array<float, 2> predict(int16_t samples[]){
  //normalizzazione del vettore
  int32_t mean = 0;
  for(int i = 0; i < 2500; i++){
    mean += samples[i];
  }
  mean /= 2500;
  for(int i = 0; i < 2500; i++){
    samples[i] -= mean;
    if(samples[i] > 3000){
      samples[i] = 3000;
    }
    else if(samples[i] < -3000){
      samples[i] = -3000;
    }
    samples[i] = (samples[i] + 3000) * (126. + 127.) / (6000) - 127;
    input->data.int8[i] = samples[i];
  }
  TfLiteStatus invoke_status = interpreter->Invoke();
  if(invoke_status != kTfLiteOk){
    error_reporter->Report("Invoke failed\n");
  }
  std::array<float, 2> res;
  res[0] = output->data.f[0]; //uscita che contiene lo score positivo (no rumore)
  res[1] = output->data.f[1]; //uscita che contiene lo score negativo (rumore)
  // se res[0] < res[1] significa presenza di rumore
  return res;
}

