#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
//#include <vector>

using namespace std;



// using a function to authomatize returning hex sector
//string get_sectors(ifstream& file, unsigned long long counter, unsigned char format_sample, int sectors_amount) { 
//
//    char byte[3] = { 0 };
//    string sectors_hex = "";
//
//    for (int i = 0; i < sectors_amount; i++) {
//
//        for (int j = 0; j < 512; j++) {
//
//            file.seekg(counter + (i * 512) + j);
//            file.read(reinterpret_cast <char*> (&format_sample), sizeof(format_sample));
//            sprintf_s(byte, "%02x", format_sample);
//
//            sectors_hex.append(byte);
//        }
//
//    }
//
//    return sectors_hex;
//};


// using a function to authomatize comparation process for usbc hex values
bool check_Nth(ifstream& file, unsigned long long counter, unsigned char format_sample, int byte_position) {

    char Nth[3] = { 0 };


    file.seekg(counter + (byte_position - 1));
    file.read(reinterpret_cast <char*> (&format_sample), sizeof(format_sample));
    sprintf_s(Nth, "%02x", format_sample);
    file.seekg(counter);

    switch (byte_position) {
        case 1: return strncmp(Nth, "55", 2); break;
        case 2: return strncmp(Nth, "53", 2); break;
        case 3: return strncmp(Nth, "42", 2); break;
        case 4: return strncmp(Nth, "43", 2); break;
        default: return false; break;
    }

};

// using this function for searching and comparing two different values that may tell us about circular rotation block length
unsigned int find_sectors_amount(ifstream& file, unsigned long long counter, unsigned char format_sample) { // 

    char sectors_byte_hex[3] = { 0 };
    char working_byte[3] = { 0 };
    string sectors_amount_hex = "";
    string bytes_amount_hex = "";
    unsigned int sectors_amount_int = 0;


    file.seekg(counter + 22);
    file.read(reinterpret_cast <char*> (&format_sample), sizeof(format_sample));
    sprintf_s(sectors_byte_hex, "%02x", format_sample);
    sectors_amount_hex.append(sectors_byte_hex);

    file.seekg(counter + 23);
    file.read(reinterpret_cast <char*> (&format_sample), sizeof(format_sample));
    sprintf_s(sectors_byte_hex, "%02x", format_sample);
    sectors_amount_hex.append(sectors_byte_hex);

    file.seekg(counter + 11);
    file.read(reinterpret_cast <char*> (&format_sample), sizeof(format_sample));
    sprintf_s(working_byte, "%02x", format_sample);
    bytes_amount_hex.append(working_byte);
    
    file.seekg(counter + 10);
    file.read(reinterpret_cast <char*> (&format_sample), sizeof(format_sample));
    sprintf_s(working_byte, "%02x", format_sample);
    bytes_amount_hex.append(working_byte);

    file.seekg(counter + 9);
    file.read(reinterpret_cast <char*> (&format_sample), sizeof(format_sample));
    sprintf_s(working_byte, "%02x", format_sample);
    bytes_amount_hex.append(working_byte);

    file.seekg(counter + 8);
    file.read(reinterpret_cast <char*> (&format_sample), sizeof(format_sample));
    sprintf_s(working_byte, "%02x", format_sample);
    bytes_amount_hex.append(working_byte);

    file.seekg(counter);

    // checking if both values give us the same result so will not touch incomplete or strange sectors to prevent even further image corruption
    if ((stoul(bytes_amount_hex, nullptr, 16) / 512) == stoul(sectors_amount_hex, nullptr, 16)) {
        
        sectors_amount_int = stoul(sectors_amount_hex, nullptr, 16);

    }

    return sectors_amount_int;
}


// using a function to convert our hex into bytes so we wont lose any data
string convert_to_bytes(string hex_string) {

    basic_string<uint8_t> bytes;


    for (size_t i = 0; i < hex_string.length(); i += 2)
    {
        uint16_t byte;
        

        // Get current pair and store in nextbyte
        string nextbyte = hex_string.substr(i, 2);

        // Put the pair into an istringstream and stream it through std::hex for
        // conversion into an integer value.
        // This will calculate the byte value of your string-represented hex value.
        // converting our bytes into hex values 
        istringstream(nextbyte) >> hex >> byte;

        // adding our values into our byte array via casting because stream does not work with uint8 directly
        bytes.push_back(static_cast<uint8_t>(byte));
    }

    // creating a string with binary values so we can output it directly into file
    string result(begin(bytes), end(bytes));

    return result;
}


// main code functionality taken outside of main function for logic construction convenience
int work_with_usbc(ifstream& disc_image_input, ofstream& disc_image_output, string path_string) {

    char current_byte[3] = { 0 };

    unsigned char char_sample;
    unsigned long long file_length = 0;
    unsigned long long i = 0;

    int usbc_counter = 0;                                              
    int usbc_rotation_block_length = 0;
    string usbc_sector = "";
    string bytes_of_usbc_sector = "";
    string rotated_sectors = "";
    string bytes_of_rotated_sectors = "";

    string current_usbc_output_byte = "";
    string current_rotated_output_byte = "";

    int usbc_output_counter = 0;
    int rotation_output_counter = 0;


    if (disc_image_input.is_open()) {

        // moving to the end of file and saving its position as a length for our current file so we could know how much bytes our file has
        disc_image_input.seekg(0, ios::end);                              
        file_length = disc_image_input.tellg();
        disc_image_input.seekg(0, ios::beg);

        // using a cycle to search for usbc
        while (file_length > i) {                                   

            disc_image_input.read(reinterpret_cast <char*> (&char_sample), sizeof(char_sample));
            sprintf_s(current_byte, "%02x", char_sample);

            // whole file formatted output inside the console window
            //if (i % 1 == 0) { cout << " "; };                     
            //if (i % 4 == 0) { cout << " "; };
            //if (i % 16 == 0) { cout << endl; };
            //if (i % 512 == 0) { cout << endl; };
            //cout << current_byte;
            //disc_image_input.seekg(++i);


            // checking each each corresponding byte one after anouther to spend less resources on wrong coincidences
            if (check_Nth(disc_image_input, i, char_sample, 1) == 0) {
                if (check_Nth(disc_image_input, i, char_sample, 2) == 0) {
                    if (check_Nth(disc_image_input, i, char_sample, 3) == 0) {
                        if (check_Nth(disc_image_input, i, char_sample, 4) == 0) {

                            usbc_rotation_block_length = find_sectors_amount(disc_image_input, i, char_sample);

                            //cout << "\n\n found usbc sector on byte " << i << ", rotation block length = " << usbc_rotation_block_length << "\n";

                            usbc_sector = "";
                            rotated_sectors = "";
                            

                            for (int usbc_sector_byte = 0; usbc_sector_byte < 512; usbc_sector_byte++) {
                                disc_image_input.seekg(i + usbc_sector_byte);
                                disc_image_input.read(reinterpret_cast <char*>(&char_sample), sizeof(char_sample));
                                
                                if (usbc_sector_byte == 0) {
                                    usbc_sector.append("44");
                                }
                                else if (usbc_sector_byte == 1) {
                                    usbc_sector.append("4F");
                                }
                                else if (usbc_sector_byte == 2) {
                                    usbc_sector.append("4E");
                                }
                                else if (usbc_sector_byte == 3) {
                                    usbc_sector.append("45");
                                }
                                else {
                                    sprintf_s(current_byte, "%02x", char_sample);
                                    usbc_sector.append(current_byte);
                                }

                            }
                            disc_image_input.seekg(i);
                            

                            for (int rotated_sector_number = 1; rotated_sector_number < usbc_rotation_block_length; rotated_sector_number++) {
                                for (int rotated_sector_byte = 0; rotated_sector_byte < 512; rotated_sector_byte++) {
                                    disc_image_input.seekg(i + (rotated_sector_number * 512) + rotated_sector_byte);
                                    disc_image_input.read(reinterpret_cast <char*>(&char_sample), sizeof(char_sample));
                                    sprintf_s(current_byte, "%02x", char_sample);
                                    rotated_sectors.append(current_byte);
                                }
                            }
                            disc_image_input.seekg(i);


                            // each of sectors console outpu for debugging purposes
                            /*cout << "\n\n usbc sector " << i << " \n\n";

                            for (auto usbc_output_symbol : usbc_sector) { 
                                
                                if (usbc_output_counter % 2 == 0) { cout << " "; }; 
                                if (usbc_output_counter % 8 == 0) { cout << " "; };
                                if (usbc_output_counter % 64 == 0) { cout << endl; };
                                if (usbc_output_counter % 1024 == 0) { cout << endl; };

                                usbc_output_counter++;

                                cout << usbc_output_symbol;

                            }

                            cout << "\n\n rotation sector " << i << " \n\n";

                            for (auto rotation_output_symbol : rotated_sectors) {

                                if (rotation_output_counter % 2 == 0) { cout << " "; };
                                if (rotation_output_counter % 8 == 0) { cout << " "; };
                                if (rotation_output_counter % 64 == 0) { cout << endl; };
                                if (rotation_output_counter % 1024 == 0) { cout << endl; };

                                rotation_output_counter++;

                                cout << rotation_output_symbol;

                            }*/


                            bytes_of_usbc_sector = convert_to_bytes(usbc_sector);
                            bytes_of_rotated_sectors = convert_to_bytes(rotated_sectors);

                            usbc_counter++;
                            

                            if (usbc_rotation_block_length > 0) {
                                disc_image_output.seekp(i);
                                disc_image_output << bytes_of_rotated_sectors;

                                disc_image_output.seekp(i + ((usbc_rotation_block_length - 1) * 512));
                                disc_image_output << bytes_of_usbc_sector;
                            }
                            else if (usbc_rotation_block_length == 0) {
                                disc_image_output.seekp(i);
                                disc_image_output << bytes_of_usbc_sector;
                            }
                            
                          
                            cout << "\n usbc sector on byte " << i << " with rotation block length of " << usbc_rotation_block_length << " has been rotated\n";

                            
                            // going to the point after current rotation block to prevent wasting time on searching inside of it
                            disc_image_input.seekg(i + (usbc_rotation_block_length * (512 + 1)));
                        }
                    }
                }

            }
            disc_image_input.seekg(i += 512);

        }
        

        cout << "\n\nTotal USBC counter: " << usbc_counter;

        
        return 0;
    }
    else {
        cerr << "Error opening file." << endl;
        return 1;
    }

}



int main() {

    //basic_iostream usbc_disc;
    ifstream path_file_input, disc_image_input; 
    ofstream disc_image_output;
    string path, file_is_correct, disc_or_img;
    int result_code;
    
    // opening the path file to get usbc-corrupted file address and name
    path_file_input.open("Path.txt");                                     

    if (path_file_input.is_open()) {
        path_file_input >> path;
        path_file_input.close();
    }
    else {
        cout << "Path.txt not found in the program directory. Please check the file." << endl;
        return 1;
    }


    // outputting path of the file we're going to work with so the user could doublecheck
    cout << "\nPath in work: " << path << endl;
    cout << "\nIs this the correct path? [y/n]: ";
    cin >> file_is_correct;
    cout << "\n";

    
    if (file_is_correct == "y" || file_is_correct == "Y" || file_is_correct == "yes" || file_is_correct == "Yes" || file_is_correct == "YES") {

        /*// asking user is it a disc or an image of a disc so we woudl know how to treat pathed object
        cout << "Is it a [d]isc or an [i]mage?: ";
        cin >> disc_or_img;
        cout << "\n";


        if (disc_or_img == "d" || disc_or_img == "D" || disc_or_img == "disc" || disc_or_img == "Disc" || disc_or_img == "DISC" || disc_or_img == "disk" || disc_or_img == "Disk" || disc_or_img == "DISK") {
            disc_image_input.open(path, ios::binary | ios::in);
            disc_image_output.open(path, ios::binary | ios::out | ios::in);
        }
        else if (disc_or_img == "i" || disc_or_img == "I" || disc_or_img == "img" || disc_or_img == "Img" || disc_or_img == "IMG" || disc_or_img == "image" || disc_or_img == "Image" || disc_or_img == "IMAGE") {
            //usbc_disc.sentry(istream);
        }
        else
        {

        }*/



        
        result_code = work_with_usbc(disc_image_input, disc_image_output, path);


        cin.get();


        disc_image_output.close();
        disc_image_input.close();

        return result_code;
    }
    else if (file_is_correct == "n" || file_is_correct == "N" || file_is_correct == "no" || file_is_correct == "No" || file_is_correct == "NO") {
        PostMessage(GetConsoleWindow(), WM_CLOSE, 0, 0);
    }

}