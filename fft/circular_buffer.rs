use std::cmp;

pub struct CircularBuffer<T>
where
    T: Copy,
{
    buffer: Vec<T>,
    read_ptr: usize,
    write_ptr: usize,
}

impl<T> CircularBuffer<T>
where
    T: Copy,
{
    /// Creates a circular buffer with a default value, and buffer length.
    pub fn new(value: T, buffer_length: usize) -> Self {
        CircularBuffer {
            buffer: vec![value; buffer_length],
            read_ptr: 0,
            write_ptr: 0,
        }
    }

    /// Offsets the read pointer by a given amount
    pub fn offset_read(&mut self, read_offset: usize) {
        self.read_ptr = (self.read_ptr + read_offset) % self.buffer.len();
    }

    /// Reads from the circular buffer at a given offset.
    pub fn read(&mut self, read_offset: i32, read_len: usize) -> Vec<T> {
        let buffer_length = self.buffer.len();
        let read_ptr = (self.read_ptr as i32 + read_offset) % buffer_length as i32;
        let read_ptr = if read_ptr < 0 {
            (buffer_length as i32 + read_ptr) as usize
        } else {
            read_ptr as usize
        };
        self.read_ptr = (read_ptr + read_len) % buffer_length;
        self.buffer
            .iter()
            .cycle()
            .skip(read_ptr)
            .take(read_len)
            .copied()
            .collect()
    }

    /// Appends the given values to the circular buffer.
    pub fn append(&mut self, data: &Vec<T>) {
        let buffer_length = self.buffer.len();
        let data_length = data.len();
        let mut trimmed;
        // handle data exceeding the length of the buffer
        let data_iter = if data_length >= buffer_length {
            let length_offset = data_length - buffer_length;
            trimmed = data[length_offset..data_length].to_vec();
            trimmed.rotate_right(length_offset);
            trimmed.iter().copied()
        } else {
            data.iter().copied()
        };
        let write_end = cmp::min(self.write_ptr + data_length, buffer_length);
        self.buffer.splice(self.write_ptr..write_end, data_iter);
        self.write_ptr = (self.write_ptr + data_length) % buffer_length;
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_circular_buffer() {
        let mut buffer = CircularBuffer::new(0.0, 5);
        let values = vec![1.0, 2.0, 3.0, 4.0, 5.0];
        buffer.append(&values);
        assert_eq!(buffer.read(0, 5), vec![1.0, 2.0, 3.0, 4.0, 5.0]);
        assert_eq!(buffer.read(2, 5), vec![3.0, 4.0, 5.0, 1.0, 2.0]);
        assert_eq!(buffer.read(0, 3), vec![3.0, 4.0, 5.0]);
        let values = vec![1.0, 2.0, 3.0, 4.0, 5.0, 6.0];
        buffer.append(&values);
        assert_eq!(buffer.read(0, 5), vec![6.0, 2.0, 3.0, 4.0, 5.0]);
        assert_eq!(
            buffer.read(2, 9),
            vec![3.0, 4.0, 5.0, 6.0, 2.0, 3.0, 4.0, 5.0, 6.0]
        );
        assert_eq!(buffer.read(-2, 3), vec![5.0, 6.0, 2.0]);
        assert_eq!(buffer.read(-7, 3), vec![6.0, 2.0, 3.0]);
    }
}
