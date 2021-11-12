include(dciIntegrationUnit)
dciIntegrationUnit(poll
        INCLUDE_DIRS include
        DEPENDS cmt)

############################################################
file(GLOB_RECURSE INC include/*)
file(GLOB_RECURSE SRC src/*)

add_library(${UNAME} SHARED ${INC} ${SRC})
dciIntegrationSetupTarget(${UNAME})
target_include_directories(${UNAME} PRIVATE src)

target_link_libraries(${UNAME} PRIVATE
    mm
    cmt
    sbs
    logger
    exception
    utils
)

############################################################
include(dciHimpl)
dciHimplMakeLayouts(${UNAME}
    INCLUDE
        src

    OUTPUT
        dci/poll/implMetaInfo.hpp

    HEADERS
        impl/descriptor.hpp
        impl/timer.hpp

    CLASSES
        dci::poll::impl::Descriptor
        dci::poll::impl::Timer

    LINK cmt exception
)

############################################################
file(GLOB_RECURSE TST test/*)
include(dciTest)
dciTest(${UNAME} noenv
    SRC ${TST}
    LINK ${UNAME} utils cmt sbs exception mm
)
