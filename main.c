#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 10000
#define MAX_COUNTIES 10000

// Structure to store county data
typedef struct
{
    char county[100];
    char state[3];

    // Education
    float education_bachelors_or_higher;
    float education_high_school_or_higher;

    // Ethnicities
    float ethnicities_white_alone;
    float ethnicities_black_alone;
    float ethnicities_asian_alone;
    float ethnicities_american_indian;
    float ethnicities_native_hawaiian;
    float ethnicities_hispanic_or_latino;
    float ethnicities_two_or_more_races;
    float ethnicities_white_alone_not_hispanic;

    // Income
    int income_median_household;
    int income_per_capita;
    float income_persons_below_poverty_level;

    // Population
    int population_2014;
} CountyData;

// trim double quotes
void trim_quotes(char *str)
{
    size_t len = strlen(str);
    if (len > 0 && str[0] == '"')
    {
        memmove(str, str + 1, len - 1);
        str[len - 2] = '\0';
    }
}

void filter_state(CountyData *data, int *count, const char *state)
{
    int new_count = 0;
    long long new_total_population = 0;

    for (int i = 0; i < *count; i++)
    {
        if (strcmp(data[i].state, state) == 0) // Only copy entries belonging to the desired state
        {
            data[new_count++] = data[i];
            new_total_population += data[i].population_2014;
        }
    }

    *count = new_count; // Update count to new filtered size
    printf("Filter: state == %s (%d entries)\n", state, *count);
}

// Check if field is valid then return that field
float get_field_value(const CountyData *entry, const char *field)
{
    if (strcmp(field, "Education.High School or Higher") == 0)
        return entry->education_high_school_or_higher;
    if (strcmp(field, "Education.Bachelor's Degree or Higher") == 0)
        return entry->education_bachelors_or_higher;
    if (strcmp(field, "Income.Median Household Income") == 0)
        return entry->income_median_household;
    if (strcmp(field, "Income.Per Capita Income") == 0)
        return entry->income_per_capita;
    if (strcmp(field, "Income.Persons Below Poverty Level") == 0)
        return entry->income_persons_below_poverty_level;
    if (strcmp(field, "Ethnicities.White Alone") == 0)
        return entry->ethnicities_white_alone;
    if (strcmp(field, "Ethnicities.Black Alone") == 0)
        return entry->ethnicities_black_alone;
    if (strcmp(field, "Ethnicities.Asian Alone") == 0)
        return entry->ethnicities_asian_alone;
    if (strcmp(field, "Ethnicities.American Indian") == 0)
        return entry->ethnicities_american_indian;
    if (strcmp(field, "Ethnicities.Native Hawaiian") == 0)
        return entry->ethnicities_native_hawaiian;
    if (strcmp(field, "Ethnicities.Hispanic or Latino") == 0)
        return entry->ethnicities_hispanic_or_latino;
    if (strcmp(field, "Ethnicities.Two or More Races") == 0)
        return entry->ethnicities_two_or_more_races;
    if (strcmp(field, "Ethnicities.White Alone Not Hispanic") == 0)
        return entry->ethnicities_white_alone_not_hispanic;

    fprintf(stderr, "Invalid field for comparison: %s\n", field);
    return -1.0;
}

// filter:<field>:<ge/le>:<number>
void filter_field(CountyData *data, int *count, const char *field, const char *operator, float value)
{
    int new_count = 0;

    for (int i = 0; i < *count; i++)
    {
        float field_value = get_field_value(&data[i], field);

        if (field_value < 0)
            continue;

        if ((strcmp(operator, "le") == 0 && field_value <= value) ||
            (strcmp(operator, "ge") == 0 && field_value >= value))
        {
            data[new_count++] = data[i]; // Retain only valid entries
        }
    }

    *count = new_count; // Update count
    printf("Filter: %s %s %.6f (%d entries)\n", field, operator, value, *count);
}

void display(const CountyData *data, int num_entries)
{
    for (int i = 0; i < num_entries; i++)
    {
        // County, Population
        printf("%s, %s\n", data[i].county, data[i].state);
        printf("\tPopulation: %d\n", data[i].population_2014);

        // Education
        printf("\tEducation:\n");
        printf("\t\t>= High School: %.2f%%\n", data[i].education_high_school_or_higher);
        printf("\t\t>= Bachelor's Degree or Higher: %.2f%%\n", data[i].education_bachelors_or_higher);

        // Ethnicity
        printf("\tEthnicity Percentages:\n");
        printf("\t\tWhite Alone: %.2f%%\n", data[i].ethnicities_white_alone);
        printf("\t\tBlack Alone: %.2f%%\n", data[i].ethnicities_black_alone);
        printf("\t\tAsian Alone: %.2f%%\n", data[i].ethnicities_asian_alone);
        printf("\t\tAmerican Indian or Alaska Native: %.2f%%\n", data[i].ethnicities_american_indian);
        printf("\t\tNative Hawaiian and Other Pacific Islander: %.2f%%\n", data[i].ethnicities_native_hawaiian);
        printf("\t\tHispanic or Latino: %.2f%%\n", data[i].ethnicities_hispanic_or_latino);
        printf("\t\tTwo or More Races: %.2f%%\n", data[i].ethnicities_two_or_more_races);
        printf("\t\tWhite Alone, not Hispanic or Latino: %.2f%%\n", data[i].ethnicities_white_alone_not_hispanic);

        // Income
        printf("\tIncome:\n");
        printf("\t\tMedian Household Income: $%d\n", data[i].income_median_household);
        printf("\t\tPer Capita Income: $%d\n", data[i].income_per_capita);
        printf("\t\tBelow Poverty Level: %.2f%%\n", data[i].income_persons_below_poverty_level);
    }
}

// Parse csv file
int parse_csv(const char *filename, CountyData *data, int *count)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        perror("Error opening file");
        return -1;
    }

    char line[MAX_LINE_LENGTH];
    int line_number = 0;
    int valid_count = 0;

    while (fgets(line, sizeof(line), file))
    {
        line_number++;

        // Skip header
        if (line_number == 1)
            continue;

        char *token;
        int field_index = 0;
        CountyData entry = {0};

        token = strtok(line, ",");
        while (token)
        {
            trim_quotes(token);

            switch (field_index)
            {
            // Each case represents index number that corresponds to the correct field in the header
            case 0:
                strncpy(entry.county, token, sizeof(entry.county) - 1);
                break;
            case 1:
                strncpy(entry.state, token, sizeof(entry.state) - 1);
                break;
            case 5:
                entry.education_bachelors_or_higher = atof(token);
                break;
            case 6:
                entry.education_high_school_or_higher = atof(token);
                break;
            case 17:
                entry.ethnicities_white_alone = atof(token);
                break;
            case 13:
                entry.ethnicities_black_alone = atof(token);
                break;
            case 12:
                entry.ethnicities_asian_alone = atof(token);
                break;
            case 11:
                entry.ethnicities_american_indian = atof(token);
                break;
            case 15:
                entry.ethnicities_native_hawaiian = atof(token);
                break;
            case 14:
                entry.ethnicities_hispanic_or_latino = atof(token);
                break;
            case 16:
                entry.ethnicities_two_or_more_races = atof(token);
                break;
            case 18:
                entry.ethnicities_white_alone_not_hispanic = atof(token);
                break;
            case 25:
                entry.income_median_household = atoi(token);
                break;
            case 26:
                entry.income_per_capita = atoi(token);
                break;
            case 27:
                entry.income_persons_below_poverty_level = atof(token);
                break;
            case 38:
                entry.population_2014 = atoi(token);
                break;
            }

            token = strtok(NULL, ",");
            field_index++;
        }

        if (strlen(entry.county) == 0 || strlen(entry.state) == 0)
        {
            fprintf(stderr, "Malformed line %d: insufficient fields\n", line_number);
            continue;
        }

        if (valid_count >= MAX_COUNTIES)
        {
            fprintf(stderr, "Error: Too many entries to load (max %d reached)\n", MAX_COUNTIES);
            break;
        }

        data[valid_count++] = entry;
    }

    fclose(file);
    *count = valid_count;
    return 0;
}

// Process operations file
void process_operations(const char *operations_file, CountyData *data, int count)
{
    FILE *file = fopen(operations_file, "r");
    if (!file)
    {
        perror("Error opening operations file");
        return;
    }

    long long total_population = 0;
    for (int i = 0; i < count; i++)
    {
        total_population += data[i].population_2014;
    }

    char line[MAX_LINE_LENGTH];

    while (fgets(line, sizeof(line), file))
    {
        // Ignore blank lines
        if (strlen(line) == 0 || isspace(line[0]))
            continue;

        if (strncmp(line, "display", 7) == 0)
        {
            display(data, count);
        }
        // Handle filter
        else if (strncmp(line, "filter:", 7) == 0)
        {
            char *filter_cmd = line + 7; // Skip the 'filter:' prefix
            char *field = strtok(filter_cmd, ":");
            char *operator= strtok(NULL, ":");
            char *number_str = strtok(NULL, ":");

            if (field && operator&& number_str)
            {
                float value = atof(number_str);
                filter_field(data, &count, field, operator, value);
            }
            else
            {
                fprintf(stderr, "Malformed filter operation: %s\n", line);
            }
        }
        // Handle filter-state:<state>
        else if (strncmp(line, "filter-state:", 13) == 0)
        {
            char *state = line + 13; // Get the state abbreviation
            // Strip the newline character, if present
            size_t len = strlen(state);
            if (len > 0 && state[len - 1] == '\n')
            {
                state[len - 1] = '\0';
            }

            filter_state(data, &count, state);
            long long total_population = 0;
            for (int i = 0; i < count; i++)
            {
                total_population += data[i].population_2014;
            }
        }
        // Handle population-total
        else if (strncmp(line, "population-total", 15) == 0)
        {
            long long total_population = 0;
            for (int i = 0; i < count; i++)
            {
                total_population += data[i].population_2014;
            }
            printf("2014 population: %lld\n", total_population);
        }
        // Handle population:<field>
        else if (strncmp(line, "population:", 10) == 0)
        {
            char *field = line + 11; // Move past the "population:" prefix
                                     // Strip the newline character, if present
            size_t len = strlen(field);
            if (len > 0 && field[len - 1] == '\n')
            {
                field[len - 1] = '\0';
            }
            float total_sub_population = 0.0;
            for (int i = 0; i < count; i++)
            {
                if (strcmp(field, "Education.Bachelor's Degree or Higher") == 0)
                    total_sub_population += (data[i].population_2014 * data[i].education_bachelors_or_higher / 100);
                else if (strcmp(field, "Education.High School or Higher") == 0)
                    total_sub_population += (data[i].population_2014 * data[i].education_high_school_or_higher / 100);
                else if (strcmp(field, "Ethnicities.American Indian and Alaska Native Alone") == 0)
                    total_sub_population += (data[i].population_2014 * data[i].ethnicities_american_indian / 100);
                else if (strcmp(field, "Ethnicities.Asian Alone") == 0)
                    total_sub_population += (data[i].population_2014 * data[i].ethnicities_asian_alone / 100);
                else if (strcmp(field, "Ethnicities.Black Alone") == 0)
                    total_sub_population += (data[i].population_2014 * data[i].ethnicities_black_alone / 100);
                else if (strcmp(field, "Ethnicities.Hispanic or Latino") == 0)
                    total_sub_population += (data[i].population_2014 * data[i].ethnicities_hispanic_or_latino / 100);
                else if (strcmp(field, "Ethnicities.Native Hawaiian and Other Pacific Islander Alone") == 0)
                    total_sub_population += (data[i].population_2014 * data[i].ethnicities_native_hawaiian / 100);
                else if (strcmp(field, "Ethnicities.Two or More Races") == 0)
                    total_sub_population += (data[i].population_2014 * data[i].ethnicities_two_or_more_races / 100);
                else if (strcmp(field, "Ethnicities.White Alone") == 0)
                    total_sub_population += (data[i].population_2014 * data[i].ethnicities_white_alone / 100);
                else if (strcmp(field, "Ethnicities.White Alone, not Hispanic or Latino") == 0)
                    total_sub_population += (data[i].population_2014 * data[i].ethnicities_white_alone_not_hispanic / 100);
                else if (strcmp(field, "Income.Persons Below Poverty Level") == 0)
                    total_sub_population += (data[i].population_2014 * data[i].income_persons_below_poverty_level / 100);
            }

            printf("2014 %s population: %.6f\n", field, total_sub_population);
        }
        // Handle percent:<field>
        else if (strncmp(line, "percent:", 8) == 0)
        {
            char *field = line + 8; // Move past "percent:"
                                    // Strip the newline character, if present
            size_t len = strlen(field);
            if (len > 0 && field[len - 1] == '\n')
            {
                field[len - 1] = '\0';
            }

            float total_sub_population = 0.0;
            for (int i = 0; i < count; i++)
            {
                if (strcmp(field, "Education.Bachelor's Degree or Higher") == 0)
                    total_sub_population += (data[i].population_2014 * data[i].education_bachelors_or_higher / 100);
                else if (strcmp(field, "Education.High School or Higher") == 0)
                    total_sub_population += (data[i].population_2014 * data[i].education_high_school_or_higher / 100);
                else if (strcmp(field, "Ethnicities.American Indian and Alaska Native Alone") == 0)
                    total_sub_population += (data[i].population_2014 * data[i].ethnicities_american_indian / 100);
                else if (strcmp(field, "Ethnicities.Asian Alone") == 0)
                    total_sub_population += (data[i].population_2014 * data[i].ethnicities_asian_alone / 100);
                else if (strcmp(field, "Ethnicities.Black Alone") == 0)
                    total_sub_population += (data[i].population_2014 * data[i].ethnicities_black_alone / 100);
                else if (strcmp(field, "Ethnicities.Hispanic or Latino") == 0)
                    total_sub_population += (data[i].population_2014 * data[i].ethnicities_hispanic_or_latino / 100);
                else if (strcmp(field, "Ethnicities.Native Hawaiian and Other Pacific Islander Alone") == 0)
                    total_sub_population += (data[i].population_2014 * data[i].ethnicities_native_hawaiian / 100);
                else if (strcmp(field, "Ethnicities.Two or More Races") == 0)
                    total_sub_population += (data[i].population_2014 * data[i].ethnicities_two_or_more_races / 100);
                else if (strcmp(field, "Ethnicities.White Alone") == 0)
                    total_sub_population += (data[i].population_2014 * data[i].ethnicities_white_alone / 100);
                else if (strcmp(field, "Ethnicities.White Alone, not Hispanic or Latino") == 0)
                    total_sub_population += (data[i].population_2014 * data[i].ethnicities_white_alone_not_hispanic / 100);
                else if (strcmp(field, "Income.Persons Below Poverty Level") == 0)
                    total_sub_population += (data[i].population_2014 * data[i].income_persons_below_poverty_level / 100);
            }

            long long total_population = 0;
            for (int i = 0; i < count; i++)
            {
                total_population += data[i].population_2014;
            }

            // Calculate percentage
            float percentage = (total_sub_population / total_population) * 100;
            printf("2014 %s percentage: %.6f\n", field, percentage);
        }
        else
        {
            fprintf(stderr, "Malformed operation\n");
        }
    }

    fclose(file);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <csv_file> <operations_file>\n", argv[0]);
        return 1;
    }

    CountyData counties[MAX_COUNTIES];
    int count = 0;

    if (parse_csv(argv[1], counties, &count) == 0)
    {
        printf("%d records loaded\n", count);
        process_operations(argv[2], counties, count);
    }

    return 0;
}
