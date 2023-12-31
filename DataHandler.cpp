#include <algorithm>
#include <random>
#include "DataHandler.h"


DataHandler::DataHandler()
{
    //dynamically allocating memory to each dataset on the heap
    dataArray =  new std::vector<Data *>;
    testData = new std::vector<Data *>;
    trainingData = new std::vector<Data *>;
    validationData = new std::vector<Data *>;
}

// free dynamically allocated memory
DataHandler::~DataHandler() = default;


//reads image data from a binary file specified by the path
void DataHandler::readFeatureVector(const std::string &path)
{
    // to store header
    uint32_t header[4];  //32 bits unsigned integer array : Magic, num of images, row size, col size
    // to keep binary data
    //Using unsigned char for raw binary data
    unsigned char bytes[4];
    //opens the file specified by the path
    FILE *f = fopen(path.c_str(), "rb");
    if (f)
    {
        //reading the first 4 bytes of header
        for (int i =0; i<4 ; i++)
        {
            // if successfully reads one element of size sizeof(bytes)
            if (fread(bytes,sizeof(bytes), 1, f))
            {
                printf("Raw bytes for header[%d]: ", i);
                for (int j = 0; j < sizeof(bytes); j++) {
                    printf("%02X ", bytes[j]);
                }
                printf("\n");

                //handle potential endianness differences between the data in the file and the system's native endianness
                header[i] = convertToLittleEndian(bytes);


            }
        }

        printf ("done getting file headers for Train Images: \n");

        std::cout << "Magic: " << header[0] << ", Num of Images: " << header[1] << std::endl;
        std::cout << "Image row size: " << header[2] << ",  Image col size: " << header[3] << std::endl;


        int imageSize = header[2] * header[3] ;
        // for all number of images in binary file
        for (int i =0; i < header[1]; i++)
        {
            Data * d = new Data();
            uint8_t element[1]; // 1 byte
            for (int j = 0; j < imageSize; j++)
            {
                //reading image data and store in element
                if(fread(element, sizeof(element), 1, f))
                {
                    d->appendToFeatureVector(element[0]);
                }
                // Error Handling
                else
                {
                    if (feof(f)){
                        // End of file reached
                        std::cerr << "Error: End of file reached while reading image data.\n";
                    }
                    else if (ferror(f)) {
                        // An error occurred during reading
                        std::cerr << "Error reading image data from file. File error occurred.\n";
                    }
                    else {
                    // Some other issue occurred
                    std::cerr << "Error reading image data from file. Unknown error.\n";
                    }
                    std::cerr << "i = " << i << ", j = " << j << ", imageSize = " << imageSize << "\n";
                    fclose(f);
                    exit(1);
                }
            }
            dataArray ->push_back(d);
        }
        printf("Successfully read and stored  features. size : %zu \n" , dataArray->size());
    }
    else
    {
        printf("File not found. \n");
        exit(1);
    }
}

void DataHandler::readFeatureLabels(const std::string &path) {

    uint32_t header[2];  // Magic | num of images
    unsigned char bytes[4];
    FILE *f = fopen(path.c_str(), "rb");
    if (f){
        for (int i=0; i<2; i++){
           if (fread(bytes, sizeof(bytes) , 1 , f)){

               printf("Raw bytes for header[%d]: ", i);
               for (int j = 0; j < sizeof(bytes); j++) {
                   printf("%02X ", bytes[j]);
               }
               printf("\n");

               header[i] = convertToLittleEndian(bytes);
           }
        }

        printf("Done getting label file header. \n");
        std::cout << "Magic: " << header[0] << ", Num of Labels: " << header[1] << std::endl;

        for (int i = 0; i < header[1]; i++ ) {
            uint8_t element[1];
            if (fread(element, sizeof(element) ,1 ,f)){
                dataArray->at(i)->setLabel(element[0]);
                //std::cout << "Label[" << i << "]: " << static_cast<int>(element[0]) << '\n';
            }
            else{
                if (feof(f)){
                    // End of file reached
                    std::cerr << "Error: End of file reached while reading image data.\n";
                }
                else if (ferror(f)) {
                    // An error occurred during reading
                    std::cerr << "Error reading image data from file. File error occurred.\n";
                }
                else {
                    // Some other issue occurred
                    std::cerr << "Error reading image data from file. Unknown error.\n";
                }
                std::cerr << "i = " << i << "\n";
                fclose(f);
                exit(1);
            }
        }
        printf("Successfully read and stored label Data. Size: %zu\n", dataArray->size());
    }
    else {
        printf("Could not find file");
    }
}



void DataHandler::splitData() {
    float trainSize = dataArray->size() * TRAIN_SET_PERCENT;
    float testSize = dataArray->size() * TEST_SET_PERCENT;

    std::shuffle(dataArray->begin(), dataArray->end(), std::mt19937(std::random_device()()));

    // Assuming TRAIN_SET_PERCENT, TEST_SET_PERCENT, and VALIDATION_SET_PERCENT are floats between 0 and 1
    int trainEnd = trainSize;
    int testEnd = trainEnd + testSize;

    trainingData->insert(trainingData->end(), dataArray->begin(), dataArray->begin() + trainEnd);
    testData->insert(testData->end(), dataArray->begin() + trainEnd, dataArray->begin() + testEnd);
    validationData->insert(validationData->end(), dataArray->begin() + testEnd, dataArray->end());

    printf("Training Data Size: %zu  \n" , trainingData->size());
    printf("Test Data Size: %zu  \n" , testData->size());
    printf("Validation Data Size: %zu  \n" , validationData->size());
}

/*
void DataHandler::countClasses() {
    int count =0;
    for (unsigned i = 0; i< dataArray->size(); i++)
    {
        // std::map<uint8_t, int> classMap;
        if (classMap.find(dataArray->at(i)->getLabel()) == classMap.end()){
            std::cout<<static_cast<int>(dataArray->at(i)->getLabel())<<std::endl;
            dataArray->at(i)->setEnumeratedLabel(count);
            count++;
        }
    }
    numClasses = count;
    printf("Successfully Extract %d unique classes. \n", numClasses);
}
*/


void DataHandler::countClasses() {
    int count = 0;

    for (unsigned i = 0; i < dataArray->size(); i++) {
        uint8_t label = dataArray->at(i)->getLabel();

        if (classMap.find(label) == classMap.end()) {
            classMap[label] = count;
            dataArray->at(i)->setEnumeratedLabel(count);
            count++;
        }
        else
        {
            // If the label already exists in classMap, set the enumerated label accordingly.
            dataArray->at(i)->setEnumeratedLabel(classMap[label]);
        }

        //std::cout << static_cast<int>(label) << std::endl;
    }
    numClasses = count;
    printf("Successfully Extract %d unique classes. \n", numClasses);
}








uint32_t DataHandler::convertToLittleEndian(const unsigned char *bytes) {

    return (uint32_t) (  (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) |(bytes[3])  ) ;
}






const std::vector<Data *> * DataHandler::getTrainingData() const {
    return trainingData;
}

const std::vector<Data *> *DataHandler::getTestData() const {
    return testData;
}

const std::vector<Data *> * DataHandler::getValidationData() const {
    return validationData ;
}

