# Description: Fetch the mojito library (https://github.com/marciomoura/mojito)

include(FetchContent)

set(FETCHCONTENT_QUIET ON)

fetchcontent_declare(
    mojito
    GIT_REPOSITORY https://github.com/marciomoura/mojito.git
    GIT_TAG main
    SYSTEM
    EXCLUDE_FROM_ALL
)

fetchcontent_makeavailable(mojito)
