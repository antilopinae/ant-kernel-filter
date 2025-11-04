#include <prometheus/exposer.h>
#include <prometheus/registry.h>
#include <prometheus/counter.h>
#include <memory>

#include <ctl/prometheus.hpp>

int StartPrometheus() {
    // HTTP сервер на порту 8080
    prometheus::Exposer exposer{"127.0.0.1:8080"};

    auto registry = std::make_shared<prometheus::Registry>();

    // Создаем счетчик
    auto &counter_family = prometheus::BuildCounter()
            .Name("my_counter")
            .Help("Пример счетчика")
            .Register(*registry);
    auto &counter = counter_family.Add({{"label", "example"}});

    // Регистрируем registry в exposer
    exposer.RegisterCollectable(registry);

    // увеличиваем счетчик
    counter.Increment();

    while (true) {
        /* держим сервер живым */
    }
}
