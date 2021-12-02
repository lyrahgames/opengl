#pragma once
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <shared_mutex>
#include <thread>
#include <unordered_map>
//
#include <lyrahgames/opengl/glfw_context.hpp>
#include <lyrahgames/opengl/glfw_window.hpp>

namespace lyrahgames::opengl {

template <typename T>
concept has_setup = requires(T& t) {
  t.setup();
};

template <typename T>
concept has_cleanup = requires(T& t) {
  t.cleanup();
};

template <typename T>
concept has_render = requires(T& t) {
  t.render();
};

template <typename T>
concept has_update = requires(T& t) {
  t.update();
};

template <typename T>
concept has_process_events = requires(T& t) {
  t.process_events();
};

template <typename T>
concept has_resize = requires(T& t, int x) {
  t.resize(x, x);
};

template <typename T>
concept has_key_callback = requires(T& t, int x) {
  t.key_callback(x, x, x, x);
};

template <typename T>
concept has_scroll_callback = requires(T& t, double x) {
  t.scroll_callback(x, x);
};

template <typename T>
class glfw_application {
  using detail = T;
  constexpr decltype(auto) detail_cast() { return static_cast<detail&>(*this); }

 public:
  glfw_application(int width = 800,
                   int height = 450,
                   czstring title = "GLFW Application");
  ~glfw_application();

  // Copying is not allowed.
  glfw_application(const glfw_application&) = delete;
  glfw_application& operator=(const glfw_application&) = delete;

  // For now, moving is not allowed.
  glfw_application(glfw_application&&) = delete;
  glfw_application& operator=(glfw_application&&) = delete;

  void run();

 protected:
  glfw_context context{};
  glfw_window window;

  thread::id runner_thread{};

  using callback_type = function<void()>;
  queue<callback_type> callback_queue{};
  mutex callback_queue_mutex{};

  void enqueue(callback_type&& callback) {
    scoped_lock lock{callback_queue_mutex};
    callback_queue.push(callback);
  }

  void process_callbacks() {
    // Only the runner thread is allowed to take callbacks.
    // Therefore independently checking for emptiness,
    // introduces no race condition.
    while (!callback_queue.empty()) {
      callback_type t;
      {
        scoped_lock lock{callback_queue_mutex};
        t = callback_queue.front();
        callback_queue.pop();
      }
      invoke(t);
    }
  }

 private:
  static unordered_map<GLFWwindow*, detail*> table;
  static shared_mutex table_mutex;

  static void enlist(glfw_application* p) {
    scoped_lock lock{table_mutex};
    table.emplace(p->window, static_cast<detail*>(p));
  }

  static void delist(glfw_application* p) {
    scoped_lock lock{table_mutex};
    table.erase(p->window);
  }

  static auto object(GLFWwindow* window) {
    shared_lock lock{table_mutex};
    return table.at(window);
  }
};

template <typename T>
unordered_map<GLFWwindow*, T*> glfw_application<T>::table{};
template <typename T>
shared_mutex glfw_application<T>::table_mutex{};

template <typename T>
glfw_application<T>::glfw_application(int width, int height, czstring title)
    : window{context, width, height, title} {
  enlist(this);
}

template <typename T>
glfw_application<T>::~glfw_application() {
  delist(this);
}

template <typename T>
void glfw_application<T>::run() {
  // We need to store the thread in which the application is run.
  runner_thread = this_thread::get_id();

  // Now, we register all given callbacks.
  // Because GLFW is a C library, only functions and
  // state-less lambdas are allowed as callbacks.
  // To get rid of this disadvantage,
  // we use a thread-safe map and the window pointer
  // to retrieve the 'this' pointer of the object.

  if constexpr (has_resize<T>)
    glfwSetFramebufferSizeCallback(
        window, [](GLFWwindow* window, int width, int height) {
          // Thread-safely retrieve pointer of application.
          const auto p = object(window);
          // If 'glfwPollEvents' has been run in the same thread as 'run'
          // then directly call the callback function.
          // Otherwise, put in the callback queue of the object
          // so that it will be called in same thread
          // as the OpenGL context has been made current.
          if (p->runner_thread == this_thread::get_id())
            p->resize(width, height);
          else
            p->enqueue([p, width, height] { p->resize(width, height); });
        });

  if constexpr (has_key_callback<T>)
    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode,
                                  int action, int mods) {
      const auto p = object(window);
      if (p->runner_thread == this_thread::get_id())
        p->key_callback(key, scancode, action, mods);
      else
        p->enqueue([p, key, scancode, action, mods] {
          p->key_callback(key, scancode, action, mods);
        });
    });

  if constexpr (has_scroll_callback<T>)
    glfwSetScrollCallback(window, [](GLFWwindow* window, double x, double y) {
      const auto p = object(window);
      if (p->runner_thread == this_thread::get_id())
        p->scroll_callback(x, y);
      else
        p->enqueue([p, x, y] { p->scroll_callback(x, y); });
    });

  // Custom Initialization
  if constexpr (has_setup<T>)  //
    detail_cast().setup();

  while (!glfwWindowShouldClose(window)) {
    // Sadly, this function polls events for all
    // windows and not only for a single window.
    // This makes using callbacks in multiple threads
    // with different OpenGL contexts very hard.
    // We make 'glfwPollEvents' register all callbacks inside a queue
    // such that these callbacks will be called
    // afterwards in the same thread 'run' was called.
    glfwPollEvents();
    process_callbacks();

    if constexpr (has_process_events<T>)  //
      detail_cast().process_events();

    if constexpr (has_update<T>)  //
      detail_cast().update();

    if constexpr (has_render<T>)  //
      detail_cast().render();

    glfwSwapBuffers(window);
  }

  if constexpr (has_cleanup<T>)  //
    detail_cast().cleanup();
}

}  // namespace lyrahgames::opengl
