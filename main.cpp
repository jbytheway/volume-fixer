#include <stdio.h>

#include <map>

#include <pulse/pulseaudio.h>

struct UserData {
  void quit(int ret) {
    mainloop_api->quit(mainloop_api, ret);
  }

  pa_mainloop_api* mainloop_api;
  std::map<uint32_t, pa_cvolume> original_volumes;
};

static void source_info_callback(
  pa_context *,
  pa_source_info const* source,
  int eol,
  void* u
)
{
  assert(eol >= 0);
  if (eol) return;

  assert(u);
  auto& userdata = *static_cast<UserData*>(u);
  printf("Source %d: %s\nDescription: %s",
    source->index, source->name, source->description);
  auto const& volumes = source->volume;
  for (int i = 0; i < volumes.channels; ++i) {
    printf("Channel %d, volume %d\n", i, volumes.values[i]);
  }
  userdata.original_volumes[source->index] = source->volume;
}

static void reset_volume_callback(
  pa_context* c,
  pa_source_info const* source,
  int eol,
  void* u
)
{
  assert(eol >= 0);
  if (eol) return;

  assert(u);
  auto& userdata = *static_cast<UserData*>(u);
  printf("Source %d: %s\nDescription: %s",
    source->index, source->name, source->description);
  auto const& original_volumes = userdata.original_volumes.at(source->index);
  auto const& volumes = source->volume;
  assert(volumes.channels == original_volumes.channels);
  bool reset = false;
  for (int i = 0; i < volumes.channels; ++i) {
    if (volumes.values[i] != original_volumes.values[i]) {
      printf("Channel %d, volume %d has changed\n", i, volumes.values[i]);
      reset = true;
      break;
    }
  }

  if (reset) {
    pa_context_set_source_volume_by_index(
      c, source->index, &original_volumes, NULL, &userdata);
  }
}

static void event_callback(
  pa_context *c,
  pa_subscription_event_type_t t,
  uint32_t idx,
  void* u
)
{
  assert(u);
  auto& userdata = *static_cast<UserData*>(u);
  if ((t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK) ==
      PA_SUBSCRIPTION_EVENT_SOURCE &&
    (t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_CHANGE)
  {
    printf("event: %d, idx: %d\n", t, idx);
    pa_context_get_source_info_by_index(
      c, idx, reset_volume_callback, &userdata);
  }
}

static void context_state_callback(pa_context *c, void *u) {
  assert(u);
  auto& userdata = *static_cast<UserData*>(u);
  assert(c);

  switch (pa_context_get_state(c)) {
    case PA_CONTEXT_CONNECTING:
    case PA_CONTEXT_AUTHORIZING:
    case PA_CONTEXT_SETTING_NAME:
      break;

    case PA_CONTEXT_READY:
      {
        fprintf(stderr, "Connection established.\n");

        pa_context_set_subscribe_callback(c, event_callback, &userdata);
        pa_context_subscribe(c, PA_SUBSCRIPTION_MASK_SOURCE, NULL, &userdata);

        pa_context_get_source_info_list(c, source_info_callback, &userdata);

        break;
      }

    case PA_CONTEXT_TERMINATED:
      userdata.quit(0);
      break;

    case PA_CONTEXT_FAILED:
    default:
      fprintf(stderr, "Connection failure: %s\n",
        pa_strerror(pa_context_errno(c)));
      userdata.quit(1);
  }
}

int main()
{
  int ret;
  UserData userdata;
  auto mainloop = pa_mainloop_new();
  assert(mainloop);
  /* Create a new connection context */
  auto mainloop_api = pa_mainloop_get_api(mainloop);
  auto context = pa_context_new(mainloop_api, "volume-fixer");
  if (!context) {
    fprintf(stderr, "pa_context_new() failed.\n");
    goto quit;
  }

  pa_context_set_state_callback(context, context_state_callback, &userdata);

  /* Connect the context */
  if (pa_context_connect(context, NULL, pa_context_flags_t(0), NULL) < 0) {
    fprintf(stderr, "pa_context_connect() failed: %s",
      pa_strerror(pa_context_errno(context)));
    goto quit;
  }

  /* Run the main loop */
  if (pa_mainloop_run(mainloop, &ret) < 0) {
    fprintf(stderr, "pa_mainloop_run() failed.\n");
    goto quit;
  }

quit:

  if (context)
    pa_context_unref(context);

  if (mainloop) {
    pa_mainloop_free(mainloop);
  }
}
