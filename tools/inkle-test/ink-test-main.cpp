#include <stdio.h>

#include "ink/story.h"
#include "ink/runner.h"
#include "ink/choice.h"

int main(int argc, char *argv[]) {
    printf("[INK] Test here...\n");

    ink::runtime::story *s = ink::runtime::story::from_file("./TheIntercept.bin");
    printf("[INK] story ptr: 0x%p\n", s);

    ink::runtime::runner run = s->new_runner();
    std::string line = run->getline();
    printf("[INK] first line = %s\n", line.c_str());

    if (run->has_choices()) {
        for (int i = 0; i < run->num_choices(); i++) {
            const ink::runtime::choice *c = run->get_choice(i);
            printf(" * %s\n", c->text());
        }
    }

    run->choose(0);

    while (!run->has_choices()) {
        line = run->getline();
        printf("[INK] next line = %s\n", line.c_str());
    }

    if (run->has_choices()) {
        for (int i = 0; i < run->num_choices(); i++) {
            const ink::runtime::choice *c = run->get_choice(i);
            printf(" * %s\n", c->text());
        }
    }

    delete s;
    s = nullptr;
    return 0;
}
