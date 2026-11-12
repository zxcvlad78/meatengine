add_library(meatengine_net STATIC
    src/net.cpp
    src/Client.cpp
    src/Network.cpp
    src/NetworkPeer.cpp
    src/Replication.cpp
    src/Rpc.cpp
    src/Serialization.cpp
    src/Server.cpp
    src/Utils.cpp
    src/setup/network.cpp
    src/hooks.cpp
    src/serialization/spatial.cpp
    src/systems/network/send.cpp
    src/systems/network/update.cpp
    src/systems/network/rpc/update.cpp
)

target_include_directories(meatengine_net PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

meatengine_set_target_options(meatengine_net)

target_link_libraries(meatengine_net PUBLIC
    meatengine_app
    meatengine_ecs
    meatengine_spatial
    GameNetworkingSockets::GameNetworkingSockets
)
