#include <future>
//
#include "application.hpp"

int main() {
  auto app_task = async(launch::async, [&] {
    application app{};
    glClearColor(0, 0.5, 0.8, 1.0);
    app.run();
  });

  auto app2_task = async(launch::async, [&] {
    application app{};
    glClearColor(0.8, 0.5, 0.0, 1.0);
    app.run();
  });

  app_task.wait();
  app2_task.wait();
}
