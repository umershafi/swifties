#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../src/api/crud_handler.h"
#include <filesystem>


class CRUDHandlerTest : public ::testing::Test {
private:
    const std::string TEST_FS_DIRECTORY = "../fs_test";

protected:
    CRUDHandler *crudHandler;

    // Variables to hold test data
    std::string entity;
    std::string data;
    int id;

public:
    // Default constructor
    CRUDHandlerTest() {}

    void SetUp() override {
        // Initialize objects before each test
        entity = "TestEntity";
        data = "Sample Data";
        crudHandler = new CRUDHandler(TEST_FS_DIRECTORY);
        // Create the test directory if it doesn't exist
        if (!std::filesystem::exists(TEST_FS_DIRECTORY))
            std::filesystem::create_directory(TEST_FS_DIRECTORY);
    }

    void TearDown() override {
        // Clean up the test directory after each test
        std::filesystem::remove_all(TEST_FS_DIRECTORY);
        delete crudHandler;
    }
};

// Test Create Operation
TEST_F(CRUDHandlerTest, CreateEntity) {
    std::string result = crudHandler->create(entity, data);
    EXPECT_EQ("{\"id\": 1}", result);

    // Verify the data is correctly written
    std::string readData = crudHandler->read(entity, 1);
    EXPECT_EQ(data, readData);
}

// Test Update Operation
TEST_F(CRUDHandlerTest, UpdateEntity) {
    int id = 1;
    std::string updatedData = "Updated Data";

    crudHandler->create(entity, data);
    bool updateResult = crudHandler->update(entity, id, updatedData);
    EXPECT_TRUE(updateResult);

    std::string readData = crudHandler->read(entity, id);
    EXPECT_EQ(updatedData, readData);
}

// Test Delete Operation
TEST_F(CRUDHandlerTest, DeleteEntity) {
    int id = 1;
    crudHandler->create(entity, data);

    bool deleteResult = crudHandler->delete_(entity, id);
    EXPECT_TRUE(deleteResult);
    EXPECT_FALSE(crudHandler->exists(entity, id));
}

// Test handling of non-existent entity updates and deletes
TEST_F(CRUDHandlerTest, NonExistentOperations) {
    int nonExistentId = 999; // Assume this ID does not exist

    // Test update on non-existent entity
    bool updateResult = crudHandler->update(entity, nonExistentId, "Some Data");
    EXPECT_FALSE(updateResult);

    // Test delete on non-existent entity
    bool deleteResult = crudHandler->delete_(entity, nonExistentId);
    EXPECT_FALSE(deleteResult);
}