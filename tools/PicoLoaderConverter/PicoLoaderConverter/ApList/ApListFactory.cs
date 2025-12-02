using CsvHelper.Configuration;
using CsvHelper;
using System.Globalization;

namespace PicoLoaderConverter.ApList;

sealed class ApListFactory
{
    private const int BINARY_AP_LIST_ENTRY_SIZE = 16;

    public ApList FromBinary(byte[] data)
    {
        int entryCount = data.Length / BINARY_AP_LIST_ENTRY_SIZE;
        var entries = new ApListEntry[entryCount];
        using (var reader = new BinaryReader(new MemoryStream(data)))
        {
            for (int i = 0; i < entryCount; i++)
            {
                entries[i] = ReadBinaryApListEntry(reader);
            }
        }

        return new ApList(entries);
    }

    public byte[] ToBinary(ApList apList)
    {
        var memoryStream = new MemoryStream();
        using var writer = new BinaryWriter(memoryStream);
        foreach (var entry in apList.Entries)
        {
            WriteBinaryApListEntry(writer, entry);
        }
        return memoryStream.ToArray();
    }

    public ApList FromCsv(string csv)
    {
        var config = new CsvConfiguration(CultureInfo.InvariantCulture)
        {
            Delimiter = ";"
        };
        using var reader = new StringReader(csv);
        using var csvReader = new CsvReader(reader, config);
        var csvEntries = csvReader
            .GetRecords<CsvApListEntry>()
            .Select(ConvertFromCvsApListEntry)
            .OrderBy(entry => entry.GameCode)
            .ThenBy(entry => entry.GameVersion)
            .ToArray();
        return new ApList(csvEntries);
    }

    public string ToCsv(ApList apList)
    {
        var config = new CsvConfiguration(CultureInfo.InvariantCulture)
        {
            Delimiter = ";"
        };
        using var writer = new StringWriter();
        using var csvWriter = new CsvWriter(writer, config);
        csvWriter.WriteRecords(
            apList.Entries
                .Select(ConvertToCvsApListEntry)
                .OrderBy(entry => entry.GameCode)
                .ThenBy(entry => entry.GameVersion));
        return writer.ToString();
    }

    private ApListEntry ReadBinaryApListEntry(BinaryReader reader)
    {
        uint gameCode = reader.ReadUInt32();
        ushort info = reader.ReadUInt16();
        byte gameVersion = (byte)(info & 0x1F);
        var dsProtectVersion = (DSProtectVersion)((info >> 5) & 0x1F);
        byte dsProtectFunctionMask = (byte)(info >> 10);
        ushort regularOverlayId = reader.ReadUInt16();
        ushort sOverlayId = reader.ReadUInt16();
        uint regularOffset = ReadUInt24(reader);
        uint sOffset = ReadUInt24(reader);
        return new ApListEntry(
            gameCode,
            gameVersion,
            dsProtectVersion,
            dsProtectFunctionMask,
            regularOverlayId,
            sOverlayId,
            regularOffset,
            sOffset);
    }

    private uint ReadUInt24(BinaryReader reader)
    {
        var bytes = reader.ReadBytes(3);
        return (uint)(bytes[0] | (bytes[1] << 8) | (bytes[2] << 16));
    }

    public void WriteBinaryApListEntry(BinaryWriter writer, ApListEntry entry)
    {
        writer.Write(entry.GameCode);
        writer.Write((ushort)(entry.GameVersion | ((byte)entry.DSProtectVersion << 5) | (entry.DSProtectFunctionMask << 10)));
        writer.Write(entry.RegularOverlayId);
        writer.Write(entry.SOverlayId);
        writer.Write((byte)(entry.RegularOffset & 0xFF));
        writer.Write((byte)((entry.RegularOffset >> 8) & 0xFF));
        writer.Write((byte)((entry.RegularOffset >> 16) & 0xFF));
        writer.Write((byte)(entry.SOffset & 0xFF));
        writer.Write((byte)((entry.SOffset >> 8) & 0xFF));
        writer.Write((byte)((entry.SOffset >> 16) & 0xFF));
    }

    private ApListEntry ConvertFromCvsApListEntry(CsvApListEntry csvApListEntry)
    {
        return new ApListEntry(
            GameCodeToUint(csvApListEntry.GameCode),
            (byte)csvApListEntry.GameVersion,
            ParseDSProtectVersion(csvApListEntry.DSProtectVersion),
            (byte)csvApListEntry.DSProtectFunctionMask,
            (ushort)csvApListEntry.RegularOverlayId,
            (ushort)csvApListEntry.SOverlayId,
            (uint)csvApListEntry.RegularOffset,
            (uint)csvApListEntry.SOffset);
    }

    private CsvApListEntry ConvertToCvsApListEntry(ApListEntry apListEntry)
    {
        return new CsvApListEntry
        {
            GameCode = $"{(char)(apListEntry.GameCode & 0xFF)}{(char)((apListEntry.GameCode >> 8) & 0xFF)}" +
                $"{(char)((apListEntry.GameCode >> 16) & 0xFF)}{(char)(apListEntry.GameCode >> 24)}",
            GameVersion = apListEntry.GameVersion,
            DSProtectVersion = FormatDSProtectVersion(apListEntry.DSProtectVersion),
            DSProtectFunctionMask = apListEntry.DSProtectFunctionMask,
            RegularOverlayId = (short)apListEntry.RegularOverlayId,
            SOverlayId = (short)apListEntry.SOverlayId,
            RegularOffset = (int)apListEntry.RegularOffset,
            SOffset = (int)apListEntry.SOffset
        };
    }

    private uint GameCodeToUint(string gameCode)
    {
        if (gameCode.Length != 4)
        {
            throw new ArgumentException(
                $"Game code '{gameCode}' is not valid. It must consist of exactly 4 characters.", nameof(gameCode));
        }
        return (uint)gameCode[0] | ((uint)gameCode[1] << 8) | ((uint)gameCode[2] << 16) | ((uint)gameCode[3] << 24);
    }

    private DSProtectVersion ParseDSProtectVersion(string dsProtectVersion)
    {
        return dsProtectVersion switch
        {
            "1.00_2" => DSProtectVersion.V1_00_2,
            "1.05" => DSProtectVersion.V1_05,
            "1.06" => DSProtectVersion.V1_06,
            "1.08" => DSProtectVersion.V1_08,
            "1.10" => DSProtectVersion.V1_10,
            "1.20" => DSProtectVersion.V1_20,
            "1.22" => DSProtectVersion.V1_22,
            "1.23" => DSProtectVersion.V1_23,
            "1.23Z" => DSProtectVersion.V1_23Z,
            "1.25" => DSProtectVersion.V1_25,
            "1.26" => DSProtectVersion.V1_26,
            "1.27" => DSProtectVersion.V1_27,
            "1.28" => DSProtectVersion.V1_28,
            "2.00" => DSProtectVersion.V2_00,
            "2.01" => DSProtectVersion.V2_01,
            "2.03" => DSProtectVersion.V2_03,
            "2.05" => DSProtectVersion.V2_05,
            "2.00s" => DSProtectVersion.V2_00s,
            "2.01s" => DSProtectVersion.V2_01s,
            "2.03s" => DSProtectVersion.V2_03s,
            "2.05s" => DSProtectVersion.V2_05s,
            _ => throw new ArgumentException(
                $"DS Protect Version '{dsProtectVersion}' could not be parsed.", nameof(dsProtectVersion))
        };
    }

    private string FormatDSProtectVersion(DSProtectVersion dsProtectVersion)
    {
        return dsProtectVersion switch
        {
            DSProtectVersion.V1_00_2 => "1.00_2",
            DSProtectVersion.V1_05 => "1.05",
            DSProtectVersion.V1_06 => "1.06",
            DSProtectVersion.V1_08 => "1.08",
            DSProtectVersion.V1_10 => "1.10",
            DSProtectVersion.V1_20 => "1.20",
            DSProtectVersion.V1_22 => "1.22",
            DSProtectVersion.V1_23 => "1.23",
            DSProtectVersion.V1_23Z => "1.23Z",
            DSProtectVersion.V1_25 => "1.25",
            DSProtectVersion.V1_26 => "1.26",
            DSProtectVersion.V1_27 => "1.27",
            DSProtectVersion.V1_28 => "1.28",
            DSProtectVersion.V2_00 => "2.00",
            DSProtectVersion.V2_01 => "2.01",
            DSProtectVersion.V2_03 => "2.03",
            DSProtectVersion.V2_05 => "2.05",
            DSProtectVersion.V2_00s => "2.00s",
            DSProtectVersion.V2_01s => "2.01s",
            DSProtectVersion.V2_03s => "2.03s",
            DSProtectVersion.V2_05s => "2.05s",
            _ => throw new ArgumentException("Invalid DS Protect Version.", nameof(dsProtectVersion))
        };
    }
}
