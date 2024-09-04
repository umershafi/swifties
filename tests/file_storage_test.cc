#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../src/api/file_storage.h"
#include <filesystem>
#include <thread>


class FileStorageTest : public ::testing::Test {
private:
  const std::string TEST_FS_DIRECTORY = "../fs_test";
  std::string base;
protected:
    FileStorage* fileStorage;
public:
  // Default constructor
  FileStorageTest() {}
  void SetUp() override {
    // Initialize objects before each test
    base = TEST_FS_DIRECTORY;
    fileStorage = new FileStorage(base);
    // Create the test directory if it doesn't exist
    if (!std::filesystem::exists(TEST_FS_DIRECTORY))
      std::filesystem::create_directory(TEST_FS_DIRECTORY);
  }
  void TearDown() override {
    // Clean up the test directory after each test
    //std::filesystem::remove_all(TEST_FS_DIRECTORY);
    delete fileStorage;
  }
};

TEST_F(FileStorageTest, ReadAfterWrite) {
    std::string test_data = "test data";
    std::string entity = "TEST";
    int id = 1;

    // Using a separate thread to write to the file
    std::thread t(&FileStorage::write, fileStorage, entity, id, test_data);
    usleep(1 * 100 * 1000);  // Sleep to allow the write operation to complete

    // Reading the data written to the file
    std::string res = fileStorage->read(entity, id);
    t.join();  // Ensure the thread completes

    // Verify that the data read matches the data written
    EXPECT_EQ(test_data, res);
}

TEST_F(FileStorageTest, OverwriteExistingFile) {
    std::string entity = "TestEntity";
    int id = 1;
    std::string initial_data = "Initial data";
    std::string new_data = "New data";

    fileStorage->write(entity, id, initial_data);
    fileStorage->write(entity, id, new_data);
    std::string readData = fileStorage->read(entity, id);

    EXPECT_EQ(new_data, readData);
}

TEST_F(FileStorageTest, WriteAfterWrite) {
    std::string entity = "TestEntity";
    int id = 1;
    std::string initial_data = "Initial data";
    std::string updated_data = "Updated data";

    // First write
    fileStorage->write(entity, id, initial_data);
    // Second write, which should overwrite the first
    fileStorage->write(entity, id, updated_data);

    // Read back the data to verify the second write was effective
    std::string readData = fileStorage->read(entity, id);
    EXPECT_EQ(updated_data, readData);
}

TEST_F(FileStorageTest, ReadAfterDelete) {
    std::string entity = "TestEntity";
    int id = 1;
    std::string test_data = "Test data to be deleted";

    fileStorage->write(entity, id, test_data);
    fileStorage->remove(entity, id); // Delete the file after writing

    // Attempt to read the deleted file, expecting an error or an empty string
    // Depending on your error handling, you might expect an exception or a specific return value
    //ASSERT_THROW(fileStorage->read(entity, id), std::runtime_error); // Change exception as needed
    // OR you could check for an empty result or specific error code if exceptions are not used
    std::string result = fileStorage->read(entity, id);
    EXPECT_TRUE(result.empty());
}